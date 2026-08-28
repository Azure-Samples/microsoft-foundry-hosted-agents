# MAF Agent Samples

This repository contains a small set of sample Microsoft Agent Framework (MAF) applications that use Microsoft Foundry and Foundry Projects. Each sample creates a simple "friendly assistant" agent and either runs it once from the console or hosts it as a long-running service.

## Projects

| Project | Language | Type | Description |
|---|---|---|---|
| `01-MAF-Agent-CS` | C# | Console app | Creates an AI agent from a Microsoft Foundry project and runs a single sample prompt, then exits. |
| `02-MAF-Agent-CS-Hosted` | C# | Hosted agent | Runs as a long-lived web service that registers a Foundry **Responses** endpoint with `AgentHost`, so Foundry can call it like any other hosted agent. |
| `03-MAF-Agent-GO` | Go | Console app | Creates and runs a Microsoft Agent Framework agent backed by a Foundry project, prints one response, then exits. |
| `04-MAF-Agent-GO-Hosted` | Go | Hosted agent | A containerized service that exposes Foundry's **Invocations** protocol (including the AG-UI contract) so it can be deployed and called as a Foundry hosted agent. |
| `MAF-Agents-Samples.slnx` | — | — | Solution file for the two C# projects. |
| `05-Foundry-Agent-CPP` | C++ | Console app | Calls a Foundry Project from C++20 through Microsoft Entra authentication and the project-scoped Responses REST API. |
| `06-Foundry-Agent-CPP-Hosted` | C++ | Hosted agent | Reuses the C++ client in a Linux container that exposes Foundry's **Invocations** protocol through cpp-httplib. |

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

- .NET 10 SDK (for `01-MAF-Agent-CS` and `02-MAF-Agent-CS-Hosted`)
- Go 1.26 SDK (for `03-MAF-Agent-GO` and `04-MAF-Agent-GO-Hosted`)
- CMake 3.25 or later, Ninja, a C++20 compiler, and vcpkg (for `05-Foundry-Agent-CPP` and `06-Foundry-Agent-CPP-Hosted`)
- Azure CLI, signed in (`az login`)
- Docker (or another OCI-compatible builder) and Azure Developer CLI (`azd`), only if you plan to deploy `04-MAF-Agent-GO-Hosted` as a container — see its own README
- Docker and `azd` are also required to deploy `06-Foundry-Agent-CPP-Hosted`
- An Azure account with permission to create or use a Microsoft Foundry project

> **Note on preview packages:** The C# samples reference preview/beta NuGet packages (`Azure.AI.Projects`, `Microsoft.Agents.AI.Foundry`, `Microsoft.Agents.AI.Foundry.Hosting`). These SDKs are under active development and their APIs may change between versions. If a sample fails to build after `dotnet restore`, check whether a newer preview package version changed an API used in `Program.cs`.

> **C++ support boundary:** Microsoft Foundry and Microsoft Agent Framework do not currently provide a first-party C++ agent SDK or hosting adapter. The C++ samples use first-party `azure-identity-cpp` for authentication and repository-owned REST and hosting adapters. See [C++ Agents with Microsoft Foundry](docs/research/cpp-agents-with-microsoft-foundry.md) for the full research and trade-off analysis.

## Dev container

The fastest way to prepare all six samples is to open the repository in its
devcontainer. It provides .NET 10, Go 1.26, C++20, CMake, Ninja, vcpkg, Azure
CLI, Azure Developer CLI, and GitHub CLI on Ubuntu 24.04.

[![Open in Dev Containers](https://img.shields.io/static/v1?label=Dev%20Containers&message=Open&color=blue&logo=visualstudiocode)](https://vscode.dev/redirect?url=vscode://ms-vscode-remote.remote-containers/cloneInVolume?url=https://github.com/Azure-Samples/microsoft-foundry-hosted-agents)

For local use, install [Docker Desktop](https://www.docker.com/products/docker-desktop/)
and the [Dev Containers extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers),
then run **Dev Containers: Reopen in Container** from the VS Code command
palette. GitHub Codespaces automatically uses the same configuration.

After the container opens, run the **Build and test all samples** task. The
first C++ configuration can take several minutes while vcpkg builds
dependencies; persistent cache volumes make subsequent rebuilds faster. The
default configuration does not mount a Docker socket or require Microsoft
Foundry credentials.

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

Build the C++ samples:

```powershell
Set-Location .\05-Foundry-Agent-CPP
cmake --preset debug
cmake --build --preset debug
Set-Location ..\06-Foundry-Agent-CPP-Hosted
cmake --preset debug
cmake --build --preset debug
```

## Run

Run the C# console sample:

```powershell
dotnet run --project .\01-MAF-Agent-CS\01-MAF-Agent-CS.csproj
```

Run the C# hosted agent sample:

```powershell
dotnet run --project .\02-MAF-Agent-CS-Hosted\02-MAF-Agent-CS-Hosted.csproj
```

Run the Go console sample — see [`03-MAF-Agent-GO/README.md`](03-MAF-Agent-GO/README.md) for full details:

```powershell
go run .\03-MAF-Agent-GO
```

Run the Go hosted agent sample — see [`04-MAF-Agent-GO-Hosted/README.md`](04-MAF-Agent-GO-Hosted/README.md) for local invocation and Foundry deployment instructions:

```powershell
Set-Location .\04-MAF-Agent-GO-Hosted
go run .
```

Run the C++ samples after building:

```powershell
.\05-Foundry-Agent-CPP\build\debug\maf_agent_cpp_05.exe
.\06-Foundry-Agent-CPP-Hosted\build\debug\maf_agent_cpp_06.exe
```

See [`05-Foundry-Agent-CPP/README.md`](05-Foundry-Agent-CPP/README.md) and [`06-Foundry-Agent-CPP-Hosted/README.md`](06-Foundry-Agent-CPP-Hosted/README.md) for configuration, Linux commands, local invocation, and deployment.

## Test

`04-MAF-Agent-GO-Hosted` includes unit tests for its HTTP handlers. Run them with:

```powershell
Set-Location .\04-MAF-Agent-GO-Hosted
go test ./...
```

The C++ samples use Catch2 tests that do not require Azure credentials:

```powershell
Set-Location .\05-Foundry-Agent-CPP
ctest --preset debug
Set-Location ..\06-Foundry-Agent-CPP-Hosted
ctest --preset debug
```

The two C# samples do not currently have automated tests; they are intended as minimal, readable starting points.

## Continuous integration

A minimal GitHub Actions workflow (`.github/workflows/build.yml`) builds the .NET solution and builds/tests the Go and C++ samples on every push and pull request to `main`. It does not require Foundry credentials since it only validates that the code compiles and unit tests pass.

## Resources

- [Microsoft Foundry documentation](https://learn.microsoft.com/azure/ai-foundry/)
- [Microsoft Foundry portal](https://ai.azure.com)
- [Microsoft Agent Framework documentation](https://learn.microsoft.com/agent-framework/overview/agent-framework-overview)
- [Microsoft Agent Framework GitHub repository](https://github.com/microsoft/agent-framework)
- [Deploy and host agents in Microsoft Foundry](https://learn.microsoft.com/azure/ai-foundry/agents/how-to/hosted-agents-overview)
- [Azure Developer CLI (`azd`) documentation](https://learn.microsoft.com/azure/developer/azure-developer-cli/overview)
- [C++ Agents with Microsoft Foundry: Current Options, Gaps, and Recommended Architecture](docs/research/cpp-agents-with-microsoft-foundry.md)
- [Azure SDK for C++](https://github.com/Azure/azure-sdk-for-cpp)
- [vcpkg C++ package manager](https://vcpkg.io)

## Notes

These samples are intended for learning and experimentation with the Microsoft Agents SDK, Microsoft Agent Framework and Microsoft Foundry integration.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
