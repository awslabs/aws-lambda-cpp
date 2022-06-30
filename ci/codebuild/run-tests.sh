#!/bin/bash

set -euo pipefail

cd $CODEBUILD_SRC_DIR
cd build
rm -rf *.zip
ninja $1
ctest --output-on-failure

