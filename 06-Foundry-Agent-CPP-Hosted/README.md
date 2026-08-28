# MAF-Agent-CPP-06

This sample reuses the client from [`MAF-Agent-CPP-05`](../MAF-Agent-CPP-05/README.md) and hosts it in a C++20 Linux container for Microsoft Foundry Hosted Agents.

The server binds to `0.0.0.0`, defaults to port `8088`, and implements:

- `GET /readiness`
- `POST /invocations` using Foundry Invocations protocol `2.0.0`
- raw UTF-8 and JSON-string prompts
- a 1 MiB request limit
- stateless requests and lazy agent initialization
- `501 Not Implemented` for AG-UI objects until streaming is implemented

See the [C++ research report](../docs/research/cpp-agents-with-microsoft-foundry.md) for the rationale behind Invocations and the current C++ capability gaps.

## Prerequisites

- The local prerequisites from [`MAF-Agent-CPP-05`](../MAF-Agent-CPP-05/README.md)
- Docker for local container builds
- Azure Developer CLI (`azd`) and the Foundry agent extensions for deployment

## Build and test locally

From this directory:

```powershell
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

Run the server:

```powershell
$env:FOUNDRY_PROJECT_ENDPOINT = "https://<resource>.services.ai.azure.com/api/projects/<project>"
$env:AZURE_AI_MODEL_DEPLOYMENT_NAME = "gpt-5-mini"
.\build\debug\maf_agent_cpp_06.exe
```

On Linux:

```bash
./build/debug/maf_agent_cpp_06
```

Check readiness and invoke it:

```powershell
Invoke-RestMethod http://localhost:8088/readiness
Invoke-RestMethod http://localhost:8088/invocations `
  -Method Post `
  -ContentType "text/plain" `
  -Body "Hello!"
```

## Build the Linux AMD64 container

The Docker build context must be the repository root because the hosted sample consumes the sibling local sample:

```powershell
docker build --platform linux/amd64 `
  -f .\MAF-Agent-CPP-06\Dockerfile `
  -t maf-agent-cpp-06 .
```

Run it with your current Azure credential environment or workload identity configuration:

```powershell
docker run --rm -p 8088:8088 `
  -e FOUNDRY_PROJECT_ENDPOINT `
  -e AZURE_AI_MODEL_DEPLOYMENT_NAME `
  maf-agent-cpp-06
```

## Deploy

Replace the Foundry Project endpoint placeholder in [`azure.yaml`](azure.yaml), then follow the repository's established `azd` Hosted Agent workflow. The manifest uses a repository-root Docker context, remote build, a non-root runtime user, and Invocations `2.0.0`.

Deployment and managed-identity behavior must be validated in your own Foundry environment; CI does not perform live Azure operations.

## Initial limitations

- Plain text responses only; AG-UI streaming is not implemented
- No tools or persistent conversation state
- Repository-owned Foundry client and hosting adapter
- Linux AMD64 container target
