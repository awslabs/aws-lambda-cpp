#!/bin/bash
set -euo pipefail

OS=${1:-}

case "$OS" in
  ubuntu|arch)
    export CC=/usr/bin/clang CXX=/usr/bin/clang++
    ;;
esac

BUILD_DIR=build-pic-check
cmake -B "$BUILD_DIR" -GNinja -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR"

"${CXX:-c++}" -shared \
  -Wl,--whole-archive "$BUILD_DIR/libaws-lambda-runtime.a" -Wl,--no-whole-archive \
  -lcurl -pthread \
  -o "$BUILD_DIR/libaws-lambda-runtime-pic-check.so"

echo "PIC check passed: static archive links into a shared object"
