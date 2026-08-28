package main

import (
	"context"
	"errors"
	"fmt"
	"log"
	"os"
	"strings"

	"github.com/Azure/azure-sdk-for-go/sdk/azidentity"
	"github.com/microsoft/agent-framework-go/agent"
	"github.com/microsoft/agent-framework-go/provider/foundryprovider"
)

const defaultModel = "gpt-5-mini"

func main() {
	if err := run(context.Background()); err != nil {
		log.Fatal(err)
	}
}

func run(ctx context.Context) error {
	endpoint := strings.TrimSpace(os.Getenv("FOUNDRY_PROJECT_ENDPOINT"))
	if endpoint == "" {
		return errors.New("FOUNDRY_PROJECT_ENDPOINT environment variable is not set")
	}

	model := strings.TrimSpace(os.Getenv("AZURE_AI_MODEL_DEPLOYMENT_NAME"))
	if model == "" {
		model = defaultModel
	}

	credential, err := azidentity.NewDefaultAzureCredential(nil)
	if err != nil {
		return fmt.Errorf("create Azure credential: %w", err)
	}

	agent := foundryprovider.NewAgent(
		endpoint,
		credential,
		foundryprovider.ModelDeployment(model),
		foundryprovider.AgentConfig{
			Instructions: "You are a friendly assistant. Keep your answers brief.",
			Config: agent.Config{
				Name: "HelloAgent",
			},
		},
	)

	response, err := agent.RunText(ctx, "Hello! Tell me a fun fact about Go.").Collect()
	if err != nil {
		return fmt.Errorf("run Foundry agent: %w", err)
	}

	fmt.Println(response)
	return nil
}
