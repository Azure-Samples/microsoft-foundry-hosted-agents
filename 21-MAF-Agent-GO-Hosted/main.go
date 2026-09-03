package main

import (
	"bytes"
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"log/slog"
	"net/http"
	"os"
	"os/signal"
	"strconv"
	"strings"
	"sync"
	"syscall"
	"time"

	"github.com/Azure/azure-sdk-for-go/sdk/azidentity"
	"github.com/microsoft/agent-framework-go/agent"
	"github.com/microsoft/agent-framework-go/provider/aguiprovider"
	"github.com/microsoft/agent-framework-go/provider/foundryprovider"
)

const (
	defaultModel          = "gpt-5-mini"
	defaultPort           = 8088
	maxInvocationBodySize = 1 << 20
)

func main() {
	logger := slog.New(slog.NewJSONHandler(os.Stdout, nil))
	ctx, stop := signal.NotifyContext(context.Background(), os.Interrupt, syscall.SIGTERM)
	defer stop()

	if err := run(ctx, logger); err != nil {
		logger.Error("agent host stopped", "error", err)
		os.Exit(1)
	}
}

func run(ctx context.Context, logger *slog.Logger) error {
	port, err := resolvePort()
	if err != nil {
		return err
	}

	mux := http.NewServeMux()
	mux.Handle("/invocations", newLazyHandler(logger, func() (http.Handler, error) {
		hostedAgent, err := newAgent(logger)
		if err != nil {
			return nil, err
		}

		return newInvocationsHandler(hostedAgent, logger), nil
	}))
	mux.HandleFunc("/readiness", readinessHandler)

	server := &http.Server{
		Addr:              fmt.Sprintf("0.0.0.0:%d", port),
		Handler:           mux,
		ReadHeaderTimeout: 5 * time.Second,
	}

	serverError := make(chan error, 1)
	go func() {
		logger.Info("agent host listening", "address", server.Addr)
		serverError <- server.ListenAndServe()
	}()

	select {
	case err := <-serverError:
		if !errors.Is(err, http.ErrServerClosed) {
			return fmt.Errorf("serve agent host: %w", err)
		}
	case <-ctx.Done():
		shutdownCtx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
		defer cancel()
		if err := server.Shutdown(shutdownCtx); err != nil {
			return fmt.Errorf("shut down agent host: %w", err)
		}
	}

	return nil
}

type textRunFunc func(context.Context, string) (string, error)

type invocationsHandler struct {
	agui    http.Handler
	logger  *slog.Logger
	runText textRunFunc
}

func newInvocationsHandler(hostedAgent *agent.Agent, logger *slog.Logger) http.Handler {
	session := &agent.Session{}
	var sessionMu sync.Mutex

	return &invocationsHandler{
		agui: aguiprovider.NewJSONHTTPHandler(hostedAgent, aguiprovider.HandlerConfig{
			Logger: logger,
		}),
		logger: logger,
		runText: func(ctx context.Context, prompt string) (string, error) {
			sessionMu.Lock()
			defer sessionMu.Unlock()

			response, err := hostedAgent.RunText(ctx, prompt, agent.WithSession(session)).Collect()
			if err != nil {
				return "", err
			}
			return response.String(), nil
		},
	}
}

func (h *invocationsHandler) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		h.agui.ServeHTTP(w, r)
		return
	}

	body, err := readInvocationBody(r.Body)
	if err != nil {
		h.logger.WarnContext(r.Context(), "read invocation input", "error", err)
		http.Error(w, err.Error(), http.StatusBadRequest)
		return
	}

	prompt, isAGUI, err := classifyInvocationBody(body)
	if err != nil {
		h.logger.WarnContext(r.Context(), "classify invocation input", "error", err)
		http.Error(w, err.Error(), http.StatusBadRequest)
		return
	}
	if isAGUI {
		r.Body = io.NopCloser(bytes.NewReader(body))
		r.ContentLength = int64(len(body))
		h.agui.ServeHTTP(w, r)
		return
	}

	output, err := h.runText(r.Context(), prompt)
	if err != nil {
		h.logger.ErrorContext(r.Context(), "run text invocation", "error", err)
		http.Error(w, "agent invocation failed", http.StatusBadGateway)
		return
	}
	if output == "" {
		h.logger.ErrorContext(r.Context(), "run text invocation", "error", "agent returned an empty response")
		http.Error(w, "agent returned an empty response", http.StatusBadGateway)
		return
	}

	w.Header().Set("Content-Type", "text/plain; charset=utf-8")
	w.Header().Set("Cache-Control", "no-store")
	w.WriteHeader(http.StatusOK)
	_, _ = w.Write([]byte(output))
}

func readInvocationBody(body io.ReadCloser) ([]byte, error) {
	if body == nil {
		return nil, errors.New("request body is required")
	}
	defer func() { _ = body.Close() }()

	data, err := io.ReadAll(io.LimitReader(body, maxInvocationBodySize+1))
	if err != nil {
		return nil, fmt.Errorf("read request body: %w", err)
	}
	if len(data) > maxInvocationBodySize {
		return nil, fmt.Errorf("request body exceeds %d bytes", maxInvocationBodySize)
	}
	return data, nil
}

func classifyInvocationBody(body []byte) (prompt string, isAGUI bool, err error) {
	trimmed := bytes.TrimSpace(body)
	if len(trimmed) == 0 {
		return "", false, errors.New("request body is required")
	}

	if json.Valid(trimmed) {
		switch trimmed[0] {
		case '{':
			var envelope struct {
				Messages json.RawMessage `json:"messages"`
			}
			if err := json.Unmarshal(trimmed, &envelope); err != nil {
				return "", false, fmt.Errorf("decode JSON invocation: %w", err)
			}
			if len(envelope.Messages) == 0 {
				return "", false, errors.New("JSON invocation must contain AG-UI messages")
			}
			return "", true, nil
		case '"':
			if err := json.Unmarshal(trimmed, &prompt); err != nil {
				return "", false, fmt.Errorf("decode JSON prompt: %w", err)
			}
		default:
			return "", false, errors.New("JSON invocation must be an AG-UI object or a string")
		}
	} else {
		if trimmed[0] == '{' || trimmed[0] == '[' {
			return "", false, errors.New("malformed JSON invocation")
		}
		prompt = string(trimmed)
	}

	prompt = strings.TrimSpace(prompt)
	if prompt == "" {
		return "", false, errors.New("prompt must not be empty")
	}
	return prompt, false, nil
}

type handlerFactory func() (http.Handler, error)

type lazyHandler struct {
	mu      sync.Mutex
	logger  *slog.Logger
	factory handlerFactory
	handler http.Handler
}

func newLazyHandler(logger *slog.Logger, factory handlerFactory) http.Handler {
	return &lazyHandler{
		logger:  logger,
		factory: factory,
	}
}

func (h *lazyHandler) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	handler, err := h.getHandler()
	if err != nil {
		h.logger.Error("initialize agent handler", "error", err)
		http.Error(w, "agent initialization failed", http.StatusInternalServerError)
		return
	}

	handler.ServeHTTP(w, r)
}

func (h *lazyHandler) getHandler() (http.Handler, error) {
	h.mu.Lock()
	defer h.mu.Unlock()

	if h.handler != nil {
		return h.handler, nil
	}

	handler, err := h.factory()
	if err != nil {
		return nil, err
	}

	h.handler = handler
	return h.handler, nil
}

func newAgent(logger *slog.Logger) (*agent.Agent, error) {
	endpoint := strings.TrimSpace(os.Getenv("FOUNDRY_PROJECT_ENDPOINT"))
	if endpoint == "" {
		return nil, errors.New("FOUNDRY_PROJECT_ENDPOINT environment variable is not set")
	}

	model := strings.TrimSpace(os.Getenv("AZURE_AI_MODEL_DEPLOYMENT_NAME"))
	if model == "" {
		model = defaultModel
	}

	credential, err := azidentity.NewDefaultAzureCredential(nil)
	if err != nil {
		return nil, fmt.Errorf("create Azure credential: %w", err)
	}

	return foundryprovider.NewAgent(
		endpoint,
		credential,
		foundryprovider.ModelDeployment(model),
		foundryprovider.AgentConfig{
			Instructions: "You are the Go hosted agent sample, running on Go 1.26 with Microsoft Agent Framework. " +
				"When greeted or asked who you are, identify yourself as the Go hosted agent. Keep your answers brief.",
			Config: agent.Config{
				Name:   "HelloAgent",
				Logger: logger,
			},
		},
	), nil
}

func resolvePort() (int, error) {
	value := strings.TrimSpace(os.Getenv("PORT"))
	if value == "" {
		return defaultPort, nil
	}

	port, err := strconv.Atoi(value)
	if err != nil || port < 1 || port > 65535 {
		return 0, fmt.Errorf("PORT must be an integer between 1 and 65535, got %q", value)
	}
	return port, nil
}

func readinessHandler(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodGet {
		w.Header().Set("Allow", http.MethodGet)
		http.Error(w, "method not allowed", http.StatusMethodNotAllowed)
		return
	}

	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	_, _ = w.Write([]byte(`{"status":"ready"}`))
}
