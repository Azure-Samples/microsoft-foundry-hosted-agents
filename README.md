# MAF Agent Samples

This repository contains a small set of sample Microsoft Agent Framework (MAF) applications that use Microsoft Foundry and Azure AI Projects. Each sample creates a simple "friendly assistant" agent and either runs it once from the console or hosts it as a long-running service.

## Projects

| Project | Language | Type | Description |
|---|---|---|---|
| `MAF-Agent-CS-01` | C# | Console app | Creates an AI agent from a Microsoft Foundry project and runs a single sample prompt, then exits. |
| `MAF-Agent-CS-02` | C# | Hosted agent | Runs as a long-lived web service that registers a Foundry **Responses** endpoint with `AgentHost`, so Foundry can call it like any other hosted agent. |
| `MAF-Agent-GO-03` | Go | Console app | Creates and runs a Microsoft Agent Framework agent backed by a Foundry project, prints one response, then exits. |
| `MAF-Agent-GO-04` | Go | Hosted agent | A containerized service that exposes Foundry's **Invocations** protocol (including the AG-UI contract) so it can be deployed and called as a Foundry hosted agent. |
| `MAF-Agents-Samples.slnx` | — | — | Solution file for the two C# projects. |

**Console app vs. hosted agent, in plain terms:**
- A **console app** is a simple, one-shot program you run locally with `dotnet run` or `go run`. It calls Foundry once, prints the answer, and exits. Use these first to confirm your Foundry project and model deployment work.
- A **hosted agent** is a long-running service (web server or container) that Foundry itself deploys and calls over HTTP using a defined protocol (`responses` or `invocations`). Hosted agents are meant to be deployed with `azd` so Foundry can invoke them repeatedly, e.g. from the Foundry playground or another application.

## Prerequisites

Before running any sample, you need a Microsoft Foundry project with a deployed chat model. If you've never used Microsoft Foundry before:

1. **Create a Microsoft Foundry resource and project.** In the [Microsoft Foundry portal](https://ai.azure.com), create a new Foundry resource (or use an existing one) and a project inside it. This gives you a **project endpoint** that looks like `https://<resource-name>.services.ai.azure.com/api/projects/<project-name>`.
2. **Deploy a chat model in that project**, for example `gpt-5-mini`. All samples in this repo default to `gpt-5-mini`, so deploying a model with that exact name lets you run every sample without changing any code. Note the **deployment name** you chose — it may differ from the underlying model name.
3. **Sign in with the Azure CLI** so the samples can authenticate: `az login`. The samples use `DefaultAzureCredential`/`AzureCliCredential`, so being signed in locally is enough — no API keys are needed.

Once you have those two values — the **project endpoint** and the **model deployment name** — you can run any sample in this repo.

Tooling prerequisites:

- .NET 10 SDK (for `MAF-Agent-CS-01` and `MAF-Agent-CS-02`)
- Go 1.26 SDK (for `MAF-Agent-GO-03` and `MAF-Agent-GO-04`)
- Azure CLI, signed in (`az login`)
- Docker (or another OCI-compatible builder) and Azure Developer CLI (`azd`), only if you plan to deploy `MAF-Agent-GO-04` as a container — see its own README
- An Azure account with permission to create or use a Microsoft Foundry project

> **Note on preview packages:** The C# samples reference preview/beta NuGet packages (`Azure.AI.Projects`, `Microsoft.Agents.AI.Foundry`, `Microsoft.Agents.AI.Foundry.Hosting`). These SDKs are under active development and their APIs may change between versions. If a sample fails to build after `dotnet restore`, check whether a newer preview package version changed an API used in `Program.cs`.

## Environment variables

All samples use the same two environment variable names:

- `FOUNDRY_PROJECT_ENDPOINT` — your Foundry project endpoint from the prerequisites step above.
- `AZURE_AI_MODEL_DEPLOYMENT_NAME` — the model deployment name in that project (defaults to `gpt-5-mini` in every sample if unset).

PowerShell:

```powershell
$env:FOUNDRY_PROJECT_ENDPOINT = "https://<your-project-endpoint>"
$env:AZURE_AI_MODEL_DEPLOYMENT_NAME = "gpt-5-mini"
```

## Build

```powershell
dotnet build .\MAF-Agents-Samples.slnx
```

## Run

Run the C# console sample:

```powershell
dotnet run --project .\MAF-Agent-CS-01\MAF-Agent-CS-01.csproj
```

Run the C# hosted agent sample:

```powershell
dotnet run --project .\MAF-Agent-CS-02\MAF-Agent-CS-02.csproj
```

Run the Go console sample — see [`MAF-Agent-GO-03/README.md`](MAF-Agent-GO-03/README.md) for full details:

```powershell
go run .\MAF-Agent-GO-03
```

Run the Go hosted agent sample — see [`MAF-Agent-GO-04/README.md`](MAF-Agent-GO-04/README.md) for local invocation and Foundry deployment instructions:

```powershell
Set-Location .\MAF-Agent-GO-04
go run .
```

## Test

`MAF-Agent-GO-04` includes unit tests for its HTTP handlers. Run them with:

```powershell
Set-Location .\MAF-Agent-GO-04
go test ./...
```

The other three samples do not currently have automated tests; they are intended as minimal, readable starting points.

## Continuous integration

A minimal GitHub Actions workflow (`.github/workflows/build.yml`) builds the .NET solution and builds/tests both Go modules on every push and pull request to `main`. It does not require Foundry credentials since it only validates that the code compiles and unit tests pass.

## Resources

- [Microsoft Foundry documentation](https://learn.microsoft.com/azure/ai-foundry/)
- [Microsoft Foundry portal](https://ai.azure.com)
- [Microsoft Agent Framework documentation](https://learn.microsoft.com/agent-framework/overview/agent-framework-overview)
- [Microsoft Agent Framework GitHub repository](https://github.com/microsoft/agent-framework)
- [Deploy and host agents in Microsoft Foundry](https://learn.microsoft.com/azure/ai-foundry/agents/how-to/hosted-agents-overview)
- [Azure Developer CLI (`azd`) documentation](https://learn.microsoft.com/azure/developer/azure-developer-cli/overview)

## Notes

These samples are intended for learning and experimentation with the Microsoft Agents SDK, Microsoft Agent Framework and Microsoft Foundry integration.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
