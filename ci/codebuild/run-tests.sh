#!/bin/bash

set -euo pipefail

cd $CODEBUILD_SRC_DIR
cd build
ninja aws-lambda-package-lambda-test-fun
aws s3 cp tests/resources/lambda-test-fun.zip s3://aws-lambda-cpp-tests/lambda-test-fun.zip
ctest --output-on-failure

