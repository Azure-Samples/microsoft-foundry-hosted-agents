# MAF-Agent-CPP-05

This C++20 console sample calls a model deployment in a Microsoft Foundry Project, prints one answer, and exits. It mirrors the local C# and Go samples.

Microsoft does not currently provide a Foundry agent SDK for C++. This sample uses first-party [`azure-identity-cpp`](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/identity/azure-identity) for `DefaultAzureCredential`, then uses a small repository-owned libcurl client for the project-scoped OpenAI Responses endpoint. See the [C++ research report](../docs/research/cpp-agents-with-microsoft-foundry.md) for alternatives and support boundaries.

## Prerequisites

- CMake 3.25 or later
- Ninja
- A C++20-capable compiler
- [vcpkg](https://vcpkg.io), with `VCPKG_ROOT` set
- Azure CLI signed in with `az login`, or another `DefaultAzureCredential` source
- A Foundry Project with a deployed model

## Configure

```powershell
$env:FOUNDRY_PROJECT_ENDPOINT = "https://<resource>.services.ai.azure.com/api/projects/<project>"
$env:AZURE_AI_MODEL_DEPLOYMENT_NAME = "gpt-5-mini"
```

The model deployment variable is optional and defaults to `gpt-5-mini`. The project endpoint must use HTTPS and retain `/api/projects/<project>`.

## Build and test

From this directory:

```powershell
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

The tests inject both the credential and HTTP transport, so they do not require Azure access.

## Run

Windows:

```powershell
.\build\debug\maf_agent_cpp_05.exe
```

Linux or macOS:

```bash
./build/debug/maf_agent_cpp_05
```

The request uses token scope `https://ai.azure.com/.default` and posts to:

```text
<FOUNDRY_PROJECT_ENDPOINT>/openai/v1/responses
```

## Initial limitations

- Non-streaming
- No tools
- No conversation state
- Repository-owned Foundry REST adapter, with no first-party C++ SDK support commitment

This sample has unit coverage designed for offline execution. A live Foundry call must be validated against your own project and permissions.
