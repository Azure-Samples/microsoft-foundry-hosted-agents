#!/usr/bin/env bash

set -euo pipefail

user_name="vscode"

mkdir -p "/home/${user_name}/.nuget/packages"
chown -R "${user_name}:${user_name}" "/home/${user_name}/.nuget"

mkdir -p /go/pkg/mod
chown -R "${user_name}:golang" /go/pkg

mkdir -p "/home/${user_name}/.cache/go-build"
chown -R "${user_name}:${user_name}" "/home/${user_name}/.cache/go-build"

mkdir -p /usr/local/vcpkg-downloads
chown -R "${user_name}:vcpkg" /usr/local/vcpkg-downloads
chmod g+rwx /usr/local/vcpkg-downloads

mkdir -p "/home/${user_name}/.cache/vcpkg/archives"
chown -R "${user_name}:${user_name}" "/home/${user_name}/.cache/vcpkg"

echo "Devcontainer cache volumes are ready."
