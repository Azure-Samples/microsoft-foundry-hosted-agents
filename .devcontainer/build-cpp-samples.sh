#!/usr/bin/env bash

set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

for sample in 05-Foundry-Agent-CPP 06-Foundry-Agent-CPP-Hosted; do
    echo "Configuring and building ${sample}..."
    pushd "${repo_root}/${sample}" >/dev/null
    cmake --preset debug
    cmake --build --preset debug
    popd >/dev/null
done

echo "C++ samples are ready."