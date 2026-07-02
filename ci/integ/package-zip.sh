#!/bin/bash
set -euo pipefail

OS=$1
MODE=${2:-default}

if [ "$OS" = "arch" ]; then
  export CC=/usr/bin/clang CXX=/usr/bin/clang++
fi

rm -rf build && mkdir build && cd build
cmake .. -GNinja \
  -DCMAKE_BUILD_TYPE=Release \
  -DENABLE_TESTS=ON

if [ "$MODE" = "no-glibc" ]; then
  ninja aws-lambda-package-lambda-test-fun-no-glibc
else
  ninja aws-lambda-package-lambda-test-fun
fi

echo "Zip package created:"
ls -la tests/resources/lambda-test-fun.zip
