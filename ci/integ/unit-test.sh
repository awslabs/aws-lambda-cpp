#!/bin/bash
set -euo pipefail

OS=$1

CMAKE_ARGS=""
if [ "$OS" = "arch" ]; then
  export CC=/usr/bin/clang CXX=/usr/bin/clang++
  CMAKE_ARGS="-DENABLE_SANITIZERS=ON"
fi

mkdir -p build && cd build
cmake .. -GNinja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DENABLE_TESTS=ON \
  $CMAKE_ARGS
ninja
ctest --output-on-failure
