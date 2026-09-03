package main

import (
	"context"
	"errors"
	"io"
	"log/slog"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"
)

func TestResolvePort(t *testing.T) {
	t.Run("uses default", func(t *testing.T) {
		t.Setenv("PORT", "")

		port, err := resolvePort()
		if err != nil {
			t.Fatalf("resolvePort() error = %v", err)
		}
		if port != defaultPort {
			t.Fatalf("resolvePort() = %d, want %d", port, defaultPort)
		}
	})

	t.Run("uses configured port", func(t *testing.T) {
		t.Setenv("PORT", "9090")

		port, err := resolvePort()
		if err != nil {
			t.Fatalf("resolvePort() error = %v", err)
		}
		if port != 9090 {
			t.Fatalf("resolvePort() = %d, want 9090", port)
		}
	})

	t.Run("rejects invalid port", func(t *testing.T) {
		t.Setenv("PORT", "70000")

		if _, err := resolvePort(); err == nil {
			t.Fatal("resolvePort() error = nil, want an error")
		}
	})
}

func TestReadinessHandler(t *testing.T) {
	t.Run("reports ready", func(t *testing.T) {
		request := httptest.NewRequest(http.MethodGet, "/readiness", nil)
		response := httptest.NewRecorder()

		readinessHandler(response, request)

		if response.Code != http.StatusOK {
			t.Fatalf("status = %d, want %d", response.Code, http.StatusOK)
		}
		if response.Body.String() != `{"status":"ready"}` {
			t.Fatalf("body = %q, want readiness JSON", response.Body.String())
		}
	})

	t.Run("rejects other methods", func(t *testing.T) {
		request := httptest.NewRequest(http.MethodPost, "/readiness", nil)
		response := httptest.NewRecorder()

		readinessHandler(response, request)

		if response.Code != http.StatusMethodNotAllowed {
			t.Fatalf("status = %d, want %d", response.Code, http.StatusMethodNotAllowed)
		}
		if response.Header().Get("Allow") != http.MethodGet {
			t.Fatalf("Allow header = %q, want %q", response.Header().Get("Allow"), http.MethodGet)
		}
	})
}

func TestLazyHandler(t *testing.T) {
	t.Run("initializes once", func(t *testing.T) {
		initializations := 0
		handler := newLazyHandler(slog.New(slog.NewTextHandler(io.Discard, nil)), func() (http.Handler, error) {
			initializations++
			return http.HandlerFunc(func(w http.ResponseWriter, _ *http.Request) {
				w.WriteHeader(http.StatusNoContent)
			}), nil
		})

		for range 2 {
			response := httptest.NewRecorder()
			handler.ServeHTTP(response, httptest.NewRequest(http.MethodPost, "/invocations", nil))
			if response.Code != http.StatusNoContent {
				t.Fatalf("status = %d, want %d", response.Code, http.StatusNoContent)
			}
		}

		if initializations != 1 {
			t.Fatalf("initializations = %d, want 1", initializations)
		}
	})

	t.Run("retries after initialization failure", func(t *testing.T) {
		initializations := 0
		handler := newLazyHandler(slog.New(slog.NewTextHandler(io.Discard, nil)), func() (http.Handler, error) {
			initializations++
			if initializations == 1 {
				return nil, errors.New("temporary initialization failure")
			}
			return http.HandlerFunc(func(w http.ResponseWriter, _ *http.Request) {
				w.WriteHeader(http.StatusNoContent)
			}), nil
		})

		firstResponse := httptest.NewRecorder()
		handler.ServeHTTP(firstResponse, httptest.NewRequest(http.MethodPost, "/invocations", nil))
		if firstResponse.Code != http.StatusInternalServerError {
			t.Fatalf("first status = %d, want %d", firstResponse.Code, http.StatusInternalServerError)
		}

		secondResponse := httptest.NewRecorder()
		handler.ServeHTTP(secondResponse, httptest.NewRequest(http.MethodPost, "/invocations", nil))
		if secondResponse.Code != http.StatusNoContent {
			t.Fatalf("second status = %d, want %d", secondResponse.Code, http.StatusNoContent)
		}
		if initializations != 2 {
			t.Fatalf("initializations = %d, want 2", initializations)
		}
	})
}

func TestInvocationsHandler(t *testing.T) {
	logger := slog.New(slog.NewTextHandler(io.Discard, nil))

	t.Run("runs plain text input", func(t *testing.T) {
		var receivedPrompt string
		handler := &invocationsHandler{
			agui: http.HandlerFunc(func(http.ResponseWriter, *http.Request) {
				t.Fatal("AG-UI handler should not be called")
			}),
			logger: logger,
			runText: func(_ context.Context, prompt string) (string, error) {
				receivedPrompt = prompt
				return "Hello from Go.", nil
			},
		}

		response := httptest.NewRecorder()
		handler.ServeHTTP(response, httptest.NewRequest(http.MethodPost, "/invocations", strings.NewReader("  hi  ")))

		if response.Code != http.StatusOK {
			t.Fatalf("status = %d, want %d", response.Code, http.StatusOK)
		}
		if receivedPrompt != "hi" {
			t.Fatalf("prompt = %q, want %q", receivedPrompt, "hi")
		}
		if response.Body.String() != "Hello from Go." {
			t.Fatalf("body = %q, want plain-text response", response.Body.String())
		}
	})

	t.Run("preserves AG-UI JSON input", func(t *testing.T) {
		const input = `{"threadId":"thread-1","runId":"run-1","messages":[]}`
		handler := &invocationsHandler{
			agui: http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
				body, err := io.ReadAll(r.Body)
				if err != nil {
					t.Fatalf("read delegated body: %v", err)
				}
				if string(body) != input {
					t.Fatalf("delegated body = %q, want %q", body, input)
				}
				w.WriteHeader(http.StatusNoContent)
			}),
			logger: logger,
			runText: func(context.Context, string) (string, error) {
				t.Fatal("plain-text runner should not be called")
				return "", nil
			},
		}

		response := httptest.NewRecorder()
		handler.ServeHTTP(response, httptest.NewRequest(http.MethodPost, "/invocations", strings.NewReader(input)))

		if response.Code != http.StatusNoContent {
			t.Fatalf("status = %d, want %d", response.Code, http.StatusNoContent)
		}
	})

	t.Run("rejects malformed JSON", func(t *testing.T) {
		handler := &invocationsHandler{
			agui:   http.NotFoundHandler(),
			logger: logger,
			runText: func(context.Context, string) (string, error) {
				t.Fatal("plain-text runner should not be called")
				return "", nil
			},
		}

		response := httptest.NewRecorder()
		handler.ServeHTTP(response, httptest.NewRequest(http.MethodPost, "/invocations", strings.NewReader(`{"messages":`)))

		if response.Code != http.StatusBadRequest {
			t.Fatalf("status = %d, want %d", response.Code, http.StatusBadRequest)
		}
	})
}
