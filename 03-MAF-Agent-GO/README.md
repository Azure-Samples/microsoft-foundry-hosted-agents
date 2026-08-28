# 03-MAF-Agent-GO

This console sample uses the Microsoft Agent Framework for Go with a Microsoft Foundry project-backed agent.

## Prerequisites

- Go 1.26 or later
- Azure CLI signed in with `az login`, or another credential supported by `DefaultAzureCredential`
- A Microsoft Foundry project endpoint and a deployed chat model

## Setup

Copy `.env.example` to a local `.env` file, then set the endpoint and model deployment. Export those values in your shell before running the sample.

```powershell
$env:FOUNDRY_PROJECT_ENDPOINT = "https://<your-project-endpoint>"
$env:AZURE_AI_MODEL_DEPLOYMENT_NAME = "gpt-5-mini"
```

## Run

```powershell
go run .
```

The sample sends one prompt to a MAF `foundryprovider` agent and writes the text response to standard output.
