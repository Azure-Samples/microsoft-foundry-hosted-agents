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

See [Prerequisites and local setup](docs/prerequisites.md) for:

- the ready-to-use dev container and Codespaces setup;
- manual setup on Linux, macOS, and Windows;
- required Microsoft Foundry resources and Azure authentication;
- Bash, Zsh, PowerShell, and Command Prompt environment-variable syntax; and
- troubleshooting for common shell, credential, model, and vcpkg errors.

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

The container automatically builds all six samples when it is first created.
This can take several minutes while vcpkg builds C++ dependencies. Persistent
cache volumes make subsequent rebuilds and recreated containers faster. After
the initialization finishes, you only need to run `az login` inside the
container, set the environment variables below, and choose an agent from the
[Run](#run) section.

The default configuration does not mount a Docker socket. Docker is unnecessary
for source builds and local runs.

## Environment variables

All samples use the same two environment variable names:

- `FOUNDRY_PROJECT_ENDPOINT` — your Foundry project endpoint from the prerequisites step above.
- `AZURE_AI_MODEL_DEPLOYMENT_NAME` — the model deployment name in that project (defaults to `gpt-5-mini` in every sample if unset).

Use syntax for your current shell. The dev container and Codespaces open Bash
by default:

```bash
export FOUNDRY_PROJECT_ENDPOINT="https://<resource>.services.ai.azure.com/api/projects/<project>"
export AZURE_AI_MODEL_DEPLOYMENT_NAME="gpt-5-mini"
```

PowerShell:

```powershell
$env:FOUNDRY_PROJECT_ENDPOINT = "https://<resource>.services.ai.azure.com/api/projects/<project>"
$env:AZURE_AI_MODEL_DEPLOYMENT_NAME = "gpt-5-mini"
```

The variables last only for the current terminal. The samples do not load
`.env` files automatically. See the [setup guide](docs/prerequisites.md#set-the-environment-variables)
for Windows Command Prompt syntax and `.env` guidance.

## Build

From the repository root, build all six samples with the commands for your
shell. In VS Code on any operating system, you can instead run the
**Build and test all samples** task.

> **Dev container and Codespaces:** The initial container setup already runs
> these builds. Run them again only after changing code or when you want to
> verify the workspace; use the VS Code task when you also want to run tests.

### Linux, macOS, dev container, or Codespaces (Bash/Zsh)

```bash
dotnet build ./MAF-Agents-Samples.slnx
(cd ./03-MAF-Agent-GO && go build ./...)
(cd ./04-MAF-Agent-GO-Hosted && go build ./...)
(cd ./05-Foundry-Agent-CPP && cmake --preset debug && cmake --build --preset debug)
(cd ./06-Foundry-Agent-CPP-Hosted && cmake --preset debug && cmake --build --preset debug)
```

### Windows (PowerShell)

```powershell
dotnet build ./MAF-Agents-Samples.slnx
Push-Location ./03-MAF-Agent-GO; go build ./...; Pop-Location
Push-Location ./04-MAF-Agent-GO-Hosted; go build ./...; Pop-Location
Push-Location ./05-Foundry-Agent-CPP; cmake --preset debug; cmake --build --preset debug; Pop-Location
Push-Location ./06-Foundry-Agent-CPP-Hosted; cmake --preset debug; cmake --build --preset debug; Pop-Location
```

## Run

Set the environment variables first, then run one sample at a time from the
repository root. The hosted samples are servers and continue running until you
press <kbd>Ctrl</kbd>+<kbd>C</kbd>; invoke each one from a second terminal.

### Linux, macOS, dev container, or Codespaces (Bash/Zsh)

```bash
dotnet run --project ./01-MAF-Agent-CS/01-MAF-Agent-CS.csproj
(cd ./03-MAF-Agent-GO && go run .)
./05-Foundry-Agent-CPP/build/debug/maf_agent_cpp_05
```

### Hosted agent sessions (Bash/Zsh)

Run these examples separately from the selected sample folder. The launch
command starts the host on port `8088` and opens Agent Inspector on port `8087`.
Use the Inspector to chat with the agent, or invoke it from a second terminal.
Later CLI invocations without `--new-session` reuse the current session.

#### C# (Responses protocol)

First terminal:

```bash
cd ./02-MAF-Agent-CS-Hosted
azd ai agent run
```

Second terminal:

```bash
cd ./02-MAF-Agent-CS-Hosted
azd ai agent invoke maf-agent-cs-02 --local --new-session --protocol responses "Hello from C#!"
```

The startup process is a web server, so logs ending with `Now listening on:
http://[::]:8088` mean it is working and waiting for the invocation. With
`azure.ai.agents` 1.0.0-beta.13, `azd` may then probe the Invocations-only route
`/invocations/docs/openapi.json` and log a non-fatal `404`; the C# sample uses
Responses, so wait for `Agent ready` and invoke it with `--protocol responses`
as shown above. If startup reports an invalid project endpoint, set
`FOUNDRY_PROJECT_ENDPOINT` to the full Foundry Project URL, including
`/api/projects/<project>`. Local C# authentication prefers the identity from
`az login`, falls back to `azd auth login`, and uses managed identity when
deployed.

#### Go (Invocations protocol)

First terminal:

```bash
cd ./04-MAF-Agent-GO-Hosted
azd ai agent run
```

Second terminal:

```bash
cd ./04-MAF-Agent-GO-Hosted
azd ai agent invoke maf-agent-go-04 --local --new-session --protocol invocations "Hello from Go!"
```

#### C++ (Invocations protocol)

First terminal:

```bash
cd ./06-Foundry-Agent-CPP-Hosted
azd ai agent run
```

Second terminal:

```bash
cd ./06-Foundry-Agent-CPP-Hosted
azd ai agent invoke maf-agent-cpp-06 --local --new-session --protocol invocations "Hello from C++!"
```

### Windows (PowerShell)

```powershell
dotnet run --project ./01-MAF-Agent-CS/01-MAF-Agent-CS.csproj
Push-Location ./03-MAF-Agent-GO; go run .; Pop-Location
./05-Foundry-Agent-CPP/build/debug/maf_agent_cpp_05.exe
```

### Hosted agent sessions (PowerShell)

Run the host command from the sample folder. It opens Agent Inspector on port
`8087`; the CLI invocation in a second terminal is optional.

| Language | First terminal: start host | Second terminal: start a fresh session |
|---|---|---|
| C# | `Set-Location ./02-MAF-Agent-CS-Hosted; azd ai agent run` | `Set-Location ./02-MAF-Agent-CS-Hosted; azd ai agent invoke --local --new-session --protocol responses "Hello from C#!"` |
| Go | `Set-Location ./04-MAF-Agent-GO-Hosted; azd ai agent run` | `Set-Location ./04-MAF-Agent-GO-Hosted; azd ai agent invoke --local --new-session --protocol invocations "Hello from Go!"` |
| C++ | `Set-Location ./06-Foundry-Agent-CPP-Hosted; azd ai agent run` | `Set-Location ./06-Foundry-Agent-CPP-Hosted; azd ai agent invoke --local --new-session --protocol invocations "Hello from C++!"` |

See the READMEs for [`03-MAF-Agent-GO`](03-MAF-Agent-GO/README.md),
[`04-MAF-Agent-GO-Hosted`](04-MAF-Agent-GO-Hosted/README.md),
[`05-Foundry-Agent-CPP`](05-Foundry-Agent-CPP/README.md), and
[`06-Foundry-Agent-CPP-Hosted`](06-Foundry-Agent-CPP-Hosted/README.md) for
sample-specific details and hosted endpoint examples.

## Test

Run all available tests from the repository root.

Bash/Zsh:

```bash
(cd ./03-MAF-Agent-GO && go test ./...)
(cd ./04-MAF-Agent-GO-Hosted && go test ./...)
(cd ./05-Foundry-Agent-CPP && ctest --preset debug)
(cd ./06-Foundry-Agent-CPP-Hosted && ctest --preset debug)
```

PowerShell:

```powershell
Push-Location ./03-MAF-Agent-GO; go test ./...; Pop-Location
Push-Location ./04-MAF-Agent-GO-Hosted; go test ./...; Pop-Location
Push-Location ./05-Foundry-Agent-CPP; ctest --preset debug; Pop-Location
Push-Location ./06-Foundry-Agent-CPP-Hosted; ctest --preset debug; Pop-Location
```

The C++ tests inject credentials and HTTP transports, so they do not require
Azure access. The C# samples do not currently have automated tests; they are
intended as minimal, readable starting points.

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
