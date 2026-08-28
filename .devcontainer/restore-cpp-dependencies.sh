#!/usr/bin/env bash

set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

for sample in 05-Foundry-Agent-CPP 06-Foundry-Agent-CPP-Hosted; do
    echo "Configuring ${sample} and restoring its vcpkg dependencies..."
    pushd "${repo_root}/${sample}" >/dev/null
    cmake --preset debug
    popd >/dev/null
done

echo "C++ dependencies are ready."
