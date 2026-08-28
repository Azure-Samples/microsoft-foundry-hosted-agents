# Copilot instructions for this repository

## Terminology

Always say **"Microsoft Foundry"**. Never use "Azure AI Foundry" (the old
product name) in code, comments, docs, or commit messages — including in
new content you write and when fixing existing content that uses the old
name. This also applies to related product nouns: prefer "Foundry Project"
over "Azure AI Project" in prose (e.g. "calls a model deployment in a
Microsoft Foundry Project").

This rule applies to prose only — do NOT rename literal SDK/package/class
identifiers that happen to contain "AI" (e.g. the `Azure.AI.Projects` NuGet
package, the `AIProjectClient` class). Those are real, versioned API names
and must stay exactly as published upstream.

## What this repo is

A set of six small, independent sample apps showing Microsoft Agent Framework
(MAF) agents backed by Microsoft Foundry, across three languages (C#, Go,
C++) and two hosting styles (console app vs. hosted agent). There is no
shared runtime code between samples — each folder is self-contained with its
own dependency manifest, build system, and README.

## Foundry agent work

This project was built with the `microsoft-foundry` skill. Before working on
or answering questions about Foundry agents (deploy, invoke, evaluate,
troubleshoot, scaffold new agents, etc.), read that skill first — see
[AGENTS.md](/d:/azure-samples/microsoft-foundry-hosted-agents/AGENTS.md).

## Folder naming convention

Folders are numbered by build/complexity order and named
`NN-<Language>-Agent-<Tech>[-Hosted]`:

| Folder | Language | Type |
|---|---|---|
| [01-MAF-Agent-CS](/d:/azure-samples/microsoft-foundry-hosted-agents/01-MAF-Agent-CS) | C# | Console app |
| [02-MAF-Agent-CS-Hosted](/d:/azure-samples/microsoft-foundry-hosted-agents/02-MAF-Agent-CS-Hosted) | C# | Hosted agent |
| [03-MAF-Agent-GO](/d:/azure-samples/microsoft-foundry-hosted-agents/03-MAF-Agent-GO) | Go | Console app |
| [04-MAF-Agent-GO-Hosted](/d:/azure-samples/microsoft-foundry-hosted-agents/04-MAF-Agent-GO-Hosted) | Go | Hosted agent |
| [05-Foundry-Agent-CPP](/d:/azure-samples/microsoft-foundry-hosted-agents/05-Foundry-Agent-CPP) | C++ | Console app |
| [06-Foundry-Agent-CPP-Hosted](/d:/azure-samples/microsoft-foundry-hosted-agents/06-Foundry-Agent-CPP-Hosted) | C++ | Hosted agent |

The C++ folders intentionally use `Foundry-Agent-CPP` (not `MAF-Agent-CPP`)
because Microsoft Foundry / Microsoft Agent Framework do not provide a
first-party C++ agent SDK — the C++ samples are repository-owned adapters,
not MAF SDK usage. See
[docs/research/cpp-agents-with-microsoft-foundry.md](/d:/azure-samples/microsoft-foundry-hosted-agents/docs/research/cpp-agents-with-microsoft-foundry.md).

**Critical policy when renaming, moving, or refactoring folders:** folder
names, file paths, and prose/doc references may be changed freely, but the
following internal/deployed identifiers must NOT be changed just because a
folder was renamed, since changing them can break existing Foundry
deployments or violate language constraints:

- `name:` / service keys in each sample's `azure.yaml` (azd deployment
  identifiers, e.g. `maf-agent-cs-02`, `maf-agent-go-04`, `maf-agent-cpp-06`)
- CMake `project(...)` names, target names, and `option(...)` names in
  `CMakeLists.txt`
- `vcpkg.json` `name` fields
- Go module import paths (`go.mod`)
- C# `RootNamespace` in `.csproj` files (C# identifiers can't start with a
  digit, so these stay like `MAF_Agent_CS_01`)

The only path-like references inside those same files that DO need to track
a rename are hard functional dependencies, e.g. `entryPoint` (dll filename)
in `azure.yaml`, the source-path argument of `add_subdirectory(...)` in
CMakeLists.txt, Dockerfile `COPY`/`WORKDIR` paths, and CI workflow matrix
values / cache-key globs in
[.github/workflows/build.yml](/d:/azure-samples/microsoft-foundry-hosted-agents/.github/workflows/build.yml).

Historical/dated docs (e.g. research reports with footnote links pinned to a
specific commit SHA) should be left as frozen snapshots, not rewritten to
match current folder names.

## Build, run, test

- **.NET**: `dotnet build .\MAF-Agents-Samples.slnx` builds both C# samples.
- **Go**: each Go sample is built/tested independently from its own folder
  (`go build ./...`, `go test ./...`); there is no top-level Go workspace.
- **C++**: each C++ sample uses its own `CMakePresets.json` (`debug` preset)
  with vcpkg for dependencies; `06-Foundry-Agent-CPP-Hosted` depends on
  `05-Foundry-Agent-CPP` via `add_subdirectory`, so `05` must be buildable on
  its own first.
- CI (`.github/workflows/build.yml`) runs all three toolchains on every push
  and PR to `main` without needing Foundry credentials — it only validates
  that code compiles and unit tests pass, not live Foundry calls.

See the root [README.md](/d:/azure-samples/microsoft-foundry-hosted-agents/README.md) for prerequisites (Foundry project endpoint, model
deployment name, tooling versions) and per-sample READMEs for
language-specific details.
