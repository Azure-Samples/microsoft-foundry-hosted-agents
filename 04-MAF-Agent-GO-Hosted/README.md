# 04-MAF-Agent-GO-Hosted

This sample hosts a Microsoft Agent Framework Go agent as a containerized Microsoft Foundry Hosted Agent. It exposes `/invocations` for plain-text chat prompts and provides `/readiness` for platform health checks. The same endpoint also supports optional Agent Framework Go AG-UI JSON requests with Server-Sent Events (SSE) responses.

Foundry direct-code deployment currently supports Python and .NET runtimes. This Go sample therefore uses the supported custom-container deployment path defined by `Dockerfile` and `azure.yaml`.

## Why this sample uses Invocations

Microsoft Foundry supports both the Responses and Invocations protocols for hosted agents:

- **Responses** is generally preferred for conversational agents. It provides OpenAI Responses API compatibility and lets Foundry manage conversation history and streaming.
- **Invocations** passes request and response data through to the container. It is intended for custom payloads and streaming protocols such as AG-UI, so the application owns the request format, response format, and conversation state.

The C# samples can use an official Foundry Responses server adapter. At the time of writing, Microsoft Agent Framework Go provides an AG-UI HTTP hosting adapter but does not provide the equivalent official Foundry Responses server adapter for Go. This sample therefore declares `invocations` version `2.0.0` in `azure.yaml` so its Foundry protocol matches the endpoint that the Go application actually implements.

For convenience, `/invocations` accepts two input formats:

1. A plain-text prompt, used by the Foundry **Chat** tab and simple CLI calls. The application manages conversation history within the hosted container session.
2. An AG-UI `RunAgentInput` JSON object, used by AG-UI clients. The response is an AG-UI event stream over SSE.

The plain-text path is a compatibility layer over Invocations; it is not an implementation of the OpenAI Responses protocol. AG-UI support is optional and is included because it is the native HTTP hosting integration currently available in Agent Framework Go. This choice should be revisited when an official Foundry Responses adapter becomes available for Go.

## Prerequisites

- Go 1.26 or later
- Docker or another OCI-compatible container builder, only for local image builds
- Azure CLI and Azure Developer CLI (`azd`)
- The `azure.ai.agents` and `azure.ai.projects` azd extensions
- An Azure account that can create or use a Microsoft Foundry project

## Run locally

Set the project endpoint and model deployment:

```powershell
$env:FOUNDRY_PROJECT_ENDPOINT = "https://<resource>.services.ai.azure.com/api/projects/<project>"
$env:AZURE_AI_MODEL_DEPLOYMENT_NAME = "<model-deployment-name>"
go run .
```

The server listens on port `8088` by default. Override it with the `PORT` environment variable.

Check readiness:

```powershell
Invoke-RestMethod http://localhost:8088/readiness
```

For a simple text response, post the prompt directly:

```powershell
Invoke-RestMethod `
    -Uri http://localhost:8088/invocations `
    -Method Post `
    -ContentType "text/plain" `
    -Body "Hello!"
```

To exercise the optional AG-UI interface:

```powershell
$body = @{
    threadId = "thread-1"
    runId = "run-1"
    state = @{}
    messages = @(
        @{
            id = "message-1"
            role = "user"
            content = "Hello! Tell me a fun fact about Go."
        }
    )
    tools = @()
    context = @()
    forwardedProps = @{}
} | ConvertTo-Json -Depth 5

Invoke-WebRequest `
    -Uri http://localhost:8088/invocations `
    -Method Post `
    -ContentType "application/json" `
    -Headers @{ Accept = "text/event-stream" } `
    -Body $body
```

## Build the container

Foundry Hosted Agents require a Linux AMD64 image:

```powershell
docker build --platform linux/amd64 -t maf-agent-go-04 .
```

The runtime container intentionally runs as root. Foundry mounts the per-session
home directory at `/home/session` as a root-owned volume, so switching this image
to a non-root user can prevent the container from reaching `/readiness`.

## Deploy to Microsoft Foundry

The manifest enables an Azure remote container build, so deployment does not require a local Docker daemon. Run these commands from this folder:

```powershell
azd provision
azd deploy
```

The `azure.yaml` file references an existing Foundry project and reads the model deployment from `AZURE_AI_MODEL_DEPLOYMENT_NAME` in the selected azd environment. Configure that value with the deployment name available in the target project before deploying.

In the Foundry playground, use the **Chat** tab for plain-text prompts. Use
**Call agent** only when you want to send a complete AG-UI payload.

For plain-text invocation from the CLI:

```powershell
azd ai agent invoke maf-agent-go-04 "Hello!" --protocol invocations
```

For the optional AG-UI interface, save a payload as `request.json`, then invoke
the Invocations endpoint:

```powershell
@{
    threadId = "thread-1"
    runId = "run-1"
    state = @{}
    messages = @(
        @{
            id = "message-1"
            role = "user"
            content = "Hello!"
        }
    )
    tools = @()
    context = @()
    forwardedProps = @{}
} | ConvertTo-Json -Depth 5 | Set-Content request.json

azd ai agent invoke maf-agent-go-04 --protocol invocations --input-file request.json
```
