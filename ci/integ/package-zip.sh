#!/bin/bash
set -euo pipefail

OS=$1

CMAKE_ARGS=""
if [ "$OS" = "arch" ]; then
  export CC=/usr/bin/clang CXX=/usr/bin/clang++
fi

rm -rf build && mkdir build && cd build
cmake .. -GNinja \
  -DCMAKE_BUILD_TYPE=Release \
  -DENABLE_TESTS=ON \
  $CMAKE_ARGS
ninja aws-lambda-package-lambda-test-fun

echo "Zip package created:"
ls -la tests/resources/lambda-test-fun.zip
