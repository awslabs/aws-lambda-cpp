#!/bin/bash
set -euo pipefail

OS=$1

CMAKE_ARGS="-DENABLE_SANITIZERS=ON"
case "$OS" in
  arch)
    export CC=/usr/bin/clang CXX=/usr/bin/clang++
    ;;
  ubuntu)
    export CC=/usr/bin/clang CXX=/usr/bin/clang++
    ;;
  alpine)
    CMAKE_ARGS=""
    ;;
esac

mkdir -p build && cd build
cmake .. -GNinja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DENABLE_TESTS=ON \
  $CMAKE_ARGS
ninja
ctest --output-on-failure
