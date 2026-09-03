# Prerequisites and local setup

This guide prepares the repository for local builds and Microsoft Foundry calls on Linux, macOS, and Windows.

## Fastest setup: dev container or Codespaces

The repository dev container includes .NET 10, Go 1.26, CMake, Ninja, a C++20 compiler, vcpkg, Azure CLI, Azure Developer CLI, and GitHub CLI. After the container opens:

1. Sign in from a terminal **inside the container** with `az login`. A sign-in on the host is not automatically shared with the container.
2. Set the environment variables for your terminal shell as described below.
3. Choose and run an agent from the repository [Run guide](../README.md#run).

During first-time container creation, lifecycle commands build all eight samples,
including restoring NuGet, Go, and vcpkg dependencies. The C++ step can take
several minutes on a cold cache. Named cache volumes retain downloads and
compiled dependencies, so later builds and recreated containers are
substantially faster. You do not need to build again before the first run. After
changing code, use the commands in the repository [Build section](../README.md#build)
or run the VS Code **Build and test all samples** task.

The default dev container does not mount a Docker daemon. Docker is not needed to build, test, or run the samples from source; it is needed only for local container-image builds. Hosted-agent deployment uses an Azure remote build.

## Microsoft Foundry resources

To make a live agent call, you need:

1. A Microsoft Foundry resource and project. You can create or select these in the [Microsoft Foundry portal](https://ai.azure.com).
2. A chat model deployment in that project. The samples default to a deployment named `gpt-5-mini`; use `AZURE_AI_MODEL_DEPLOYMENT_NAME` when your deployment has another name.
3. The project endpoint, in this exact form:

   `https://<resource>.services.ai.azure.com/api/projects/<project>`

4. An Azure identity with permission to use the project and model deployment.
5. An Azure CLI session in the environment where the sample runs.

Confirm the active Azure account:

```bash
az account show --output table
```

If it reports that no account is available, run `az login` (or `az login --use-device-code` in a terminal that cannot open a browser).

No API key is required. The samples authenticate with Azure Identity credentials.

## Tooling for a manual setup

Install only the toolchains for the samples you plan to use:

| Samples | Required tools |
|---|---|
| C# (`01`, `02`, `03`, `04`) | [.NET 10 SDK](https://dotnet.microsoft.com/download/dotnet/10.0) |
| Go (`20`, `21`) | [Go 1.26 or later](https://go.dev/doc/install) |
| C++ (`40`, `41`) | [CMake 3.25+](https://cmake.org/download/), [Ninja](https://ninja-build.org/), a C++20 compiler, and [vcpkg](https://learn.microsoft.com/vcpkg/get_started/get-started) with `VCPKG_ROOT` set |
| All live calls | [Azure CLI](https://learn.microsoft.com/cli/azure/install-azure-cli), signed in with `az login` |
| Hosted-agent deployment | [Azure Developer CLI](https://learn.microsoft.com/azure/developer/azure-developer-cli/install-azd) |
| Optional local container builds | [Docker Desktop](https://www.docker.com/products/docker-desktop/) or another OCI-compatible builder |

On Windows, use PowerShell 7+ and a Visual Studio C++ toolchain for the C++ samples. On Linux, use GCC or Clang. On macOS, use Apple Clang from Xcode Command Line Tools.

Verify the installed tools as applicable:

```bash
dotnet --version
go version
cmake --version
ninja --version
vcpkg version
az version
azd version
```

## Set the environment variables

Environment-variable syntax is shell-specific. Use the block matching the terminal you are actually using.

### Bash or Zsh (Linux, macOS, dev container, Codespaces)

```bash
export FOUNDRY_PROJECT_ENDPOINT="https://<resource>.services.ai.azure.com/api/projects/<project>"
export AZURE_AI_MODEL_DEPLOYMENT_NAME="gpt-5-mini"
```

### PowerShell (Windows, Linux, or macOS)

```powershell
$env:FOUNDRY_PROJECT_ENDPOINT = "https://<resource>.services.ai.azure.com/api/projects/<project>"
$env:AZURE_AI_MODEL_DEPLOYMENT_NAME = "gpt-5-mini"
```

### Windows Command Prompt

```batch
set "FOUNDRY_PROJECT_ENDPOINT=https://<resource>.services.ai.azure.com/api/projects/<project>"
set "AZURE_AI_MODEL_DEPLOYMENT_NAME=gpt-5-mini"
```

These values apply only to the current terminal session. Set them again in each new terminal before running a sample.

The repository includes `.env.example` files as templates, but the samples do **not** load `.env` files automatically. In Bash/Zsh, you can load an edited file explicitly with `set -a; source <sample>/.env; set +a`. Otherwise, use the shell commands above.

## Common setup failures

- `$env:FOUNDRY_PROJECT_ENDPOINT: command not found`: PowerShell syntax was entered in Bash. Use `export FOUNDRY_PROJECT_ENDPOINT=...` instead.
- `FOUNDRY_PROJECT_ENDPOINT is not set`: the variable was set in another terminal, shell, or host environment. Set it in the terminal that launches the sample.
- Authentication asks for credentials or says Azure CLI is unavailable: run `az login` in the same host, container, or Codespace where the sample runs.
- Model deployment not found: set `AZURE_AI_MODEL_DEPLOYMENT_NAME` to the deployment name, which can differ from the underlying model name.
- CMake cannot find vcpkg: set `VCPKG_ROOT` to the vcpkg installation directory and open a new terminal.

Continue with the cross-platform build and run commands in the [repository README](../README.md#build).
