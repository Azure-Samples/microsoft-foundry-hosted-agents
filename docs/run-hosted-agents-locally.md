# Run the hosted agents locally

This guide runs the C#, Go, and C++ hosted-agent samples from source with
Microsoft Foundry Agent Inspector or a second terminal. Run one hosted agent at
a time because every sample uses port `8088` and Inspector uses port `8087`.

## Before you start

Complete the [prerequisites](prerequisites.md), sign in with Azure CLI and the
Azure Developer CLI, and set these values in the terminal that will launch the
agent:

```bash
az login
azd auth login
```

```bash
export FOUNDRY_PROJECT_ENDPOINT="https://<resource>.services.ai.azure.com/api/projects/<project>"
export AZURE_AI_MODEL_DEPLOYMENT_NAME="gpt-5-mini"
```

Install the required Azure Developer CLI extensions once:

```bash
azd extension install azure.ai.agents
azd extension install azure.ai.projects
```

Build the repository before the first run or after changing source code. See the
[root build instructions](../README.md#build), or run the VS Code
**Build and test all samples** task.

## Use two terminals

For each sample:

1. In terminal 1, change to the hosted-agent folder and run the launch command.
2. Wait until the server reports that it is listening on port `8088` and the
   agent is ready. Agent Inspector opens at <http://127.0.0.1:8087/>.
3. Test in Inspector, or use the matching command in terminal 2.
4. Press <kbd>Ctrl</kbd>+<kbd>C</kbd> in terminal 1 before starting another sample.

The first CLI request starts a fresh local session. The second request reuses
that session. For the C++ sample, the HTTP adapter is intentionally stateless,
so session reuse does not add model conversation memory.

## C# hosted agent

The C# sample runs on **.NET 10**, uses Microsoft Agent Framework, and exposes
the **Responses** protocol.

### Terminal 1

```bash
cd ./02-MAF-Agent-CS-Hosted
azd ai agent run
```

### Agent Inspector

Keep **Responses Protocol** selected and send `hi` in the chat box.

### Terminal 2

```bash
cd ./02-MAF-Agent-CS-Hosted
azd ai agent invoke maf-agent-cs-02 --local --new-session --new-conversation --protocol responses "hi"
azd ai agent invoke maf-agent-cs-02 --local --protocol responses "who are you"
```

A non-fatal `404` for `/invocations/docs/openapi.json` during startup can be
ignored because this sample uses Responses rather than Invocations.

## C# Harness hosted agent

The C# Harness sample runs on **.NET 10**, uses the Microsoft Agent Framework
Harness (`HarnessAgent`), and exposes the **Responses** protocol.

### Terminal 1

```bash
cd ./04-MAF-Agent-CS-Harness-Hosted
azd ai agent run
```

### Agent Inspector

Keep **Responses Protocol** selected and send `hi` in the chat box.

### Terminal 2

```bash
cd ./04-MAF-Agent-CS-Harness-Hosted
azd ai agent invoke maf-agent-cs-04 --local --new-session --new-conversation --protocol responses "hi"
azd ai agent invoke maf-agent-cs-04 --local --protocol responses "who are you"
```

## Go hosted agent

The Go sample runs on **Go 1.26**, uses Microsoft Agent Framework for Go, and
exposes the **Invocations** protocol with AG-UI.

### Terminal 1

```bash
cd ./21-MAF-Agent-GO-Hosted
azd ai agent run --start-command "go run ."
```

The explicit start command is required because `azd` does not currently
auto-detect this Go sample.

### Agent Inspector

Select **Invocations Protocol**, open **Request settings**, set
**Content-Type** to `text/plain`, enter `hi`, and select **Send**. Use
`application/json` only when you specifically want to inspect the AG-UI
request and event contracts.

### Terminal 2

```bash
cd ./21-MAF-Agent-GO-Hosted
azd ai agent invoke maf-agent-go-21 --local --new-session --protocol invocations "hi"
azd ai agent invoke maf-agent-go-21 --local --protocol invocations "who are you"
```

## C++ hosted agent

The C++ sample runs on **C++20** with a repository-owned Microsoft Foundry REST
client and hosting adapter. It exposes the **Invocations** protocol with AG-UI
text events.

### Terminal 1

```bash
cd ./41-Foundry-Agent-CPP-Hosted
azd ai agent run --start-command "./build/debug/maf_agent_cpp_41"
```

The explicit start command is required because `azd` does not auto-detect CMake
executables.

### Agent Inspector

Select **Invocations Protocol**, open **Request settings**, set
**Content-Type** to `text/plain`, enter `hi`, and select **Send**. The Inspector
protocol selection is shared across local agents and may initially remain set
to Responses.

### Terminal 2

```bash
cd ./41-Foundry-Agent-CPP-Hosted
azd ai agent invoke maf-agent-cpp-41 --local --new-session --protocol invocations "hi"
azd ai agent invoke maf-agent-cpp-41 --local --protocol invocations "who are you"
```

## PowerShell navigation

The `azd` launch and invoke arguments are the same in PowerShell. Replace each
Bash `cd ./<folder>` command with `Set-Location ./<folder>`, and set environment
variables with `$env:NAME = "value"` as shown in the
[prerequisites](prerequisites.md#set-the-environment-variables).

## Troubleshooting

- **Port 8088 is already in use:** stop the previous agent with
  <kbd>Ctrl</kbd>+<kbd>C</kbd> before launching the next one.
- **Inspector shows the wrong protocol:** choose Responses for C# and
  Invocations for Go or C++ from the protocol selector.
- **Project endpoint is rejected:** use the complete HTTPS project endpoint,
  including `/api/projects/<project>`.
- **Authentication fails:** run `az login` and `azd auth login` in the same
  host, dev container, or Codespace where the agent runs.
- **C++ executable is missing:** configure and build sample `06` with its
  `debug` CMake preset before launching it.
