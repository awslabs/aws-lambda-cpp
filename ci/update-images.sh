#!/bin/bash

set -euo pipefail

PRJ_ROOT=$(git rev-parse --show-toplevel)
ECR_NAME=${ECR_NAME:-aws-lambda-cpp-ci}
REGION=${AWS_DEFAULT_REGION:-us-west-2}
ACCOUNT_ID=$(aws sts get-caller-identity --output text --query "Account")

aws ecr get-login-password --region $REGION | docker login --username AWS --password-stdin $ACCOUNT_ID.dkr.ecr.$REGION.amazonaws.com

# on Linux, if buildx is giving trouble - run:
# docker run --rm --privileged multiarch/qemu-user-static --reset -p yes

build-and-push () {
    TAG=$ACCOUNT_ID.dkr.ecr.$REGION.amazonaws.com/$ECR_NAME:$1-$(echo $2 | sed 's|/|-|g')
    docker build --platform $2 -t $TAG -f "$PRJ_ROOT/ci/docker/$1" .
    docker push $TAG
}

if [[ $(arch) == "aarch64" ]]; then
  build-and-push amazon-linux-2       linux/arm64
else
  build-and-push ubuntu-linux-18.04   linux/amd64
  build-and-push alpine-linux-3.15    linux/amd64
  build-and-push amazon-linux-2018.03 linux/amd64
  build-and-push amazon-linux-2       linux/amd64
  build-and-push arch-linux           linux/amd64
fi
