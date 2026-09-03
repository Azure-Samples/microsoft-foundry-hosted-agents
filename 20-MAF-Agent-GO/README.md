# 20-MAF-Agent-GO

This console sample uses the Microsoft Agent Framework for Go with a Microsoft Foundry project-backed agent.

## Prerequisites

- Go 1.26 or later
- Azure CLI signed in with `az login`, or another credential supported by `DefaultAzureCredential`
- A Microsoft Foundry project endpoint and a deployed chat model

See the repository's [prerequisites and local setup guide](../docs/prerequisites.md)
for dev-container and operating-system-specific setup.

## Setup

Set the endpoint and model deployment in the terminal that will run the sample.
The sample does not load `.env` automatically.

Bash/Zsh:

```bash
export FOUNDRY_PROJECT_ENDPOINT="https://<resource>.services.ai.azure.com/api/projects/<project>"
export AZURE_AI_MODEL_DEPLOYMENT_NAME="gpt-5-mini"
```

PowerShell:

```powershell
$env:FOUNDRY_PROJECT_ENDPOINT = "https://<resource>.services.ai.azure.com/api/projects/<project>"
$env:AZURE_AI_MODEL_DEPLOYMENT_NAME = "gpt-5-mini"
```

## Run

```bash
go run .
```

The sample sends one prompt to a MAF `foundryprovider` agent and writes the text response to standard output.
