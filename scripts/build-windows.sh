#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")"/.. && pwd)"
BUILD_DIR="$ROOT_DIR/build/windows-x86_64"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_TOOLCHAIN_FILE="$ROOT_DIR/toolchains/mingw-w64-x86_64.cmake" \
      "$ROOT_DIR" | cat

cmake --build . -j$(nproc) | cat
echo "Windows binary at: $BUILD_DIR/media-converter-windows-x86_64.exe"


