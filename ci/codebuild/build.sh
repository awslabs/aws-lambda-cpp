#!/bin/bash

set -euo pipefail

# build the lambda-runtime
cd $CODEBUILD_SRC_DIR
mkdir build
cd build
cmake .. -GNinja -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/install -DENABLE_TESTS=ON $@
ninja
ninja aws-lambda-package-lambda-test-fun
aws s3 cp tests/resources/lambda-test-fun.zip s3://aws-lambda-cpp-tests/lambda-test-fun.zip
ctest --output-on-failure

