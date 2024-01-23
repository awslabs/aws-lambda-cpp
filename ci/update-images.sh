#!/bin/bash

set -euox pipefail

PRJ_ROOT=$(git rev-parse --show-toplevel)
ECR_NAME=${ECR_NAME:-aws-lambda-cpp-ci}
REGION=${AWS_DEFAULT_REGION:-us-west-2}
ACCOUNT_ID=$(aws sts get-caller-identity --output text --query "Account")

aws ecr get-login-password --region $REGION | docker login --username AWS --password-stdin $ACCOUNT_ID.dkr.ecr.$REGION.amazonaws.com

# on Linux, if buildx is giving trouble - run:
# docker run --rm --privileged multiarch/qemu-user-static --reset -p yes

tag () {
    echo $ACCOUNT_ID.dkr.ecr.$REGION.amazonaws.com/$ECR_NAME:$1-$(echo $2 | sed 's|/|-|g')
}

build-and-push () {
    docker build --platform $2 -t $(tag $1 $2) -f "$PRJ_ROOT/ci/docker/$1" .
    docker push $(tag $1 $2)
}

build () {
    docker build --platform $2 --target $1 -t $(tag $1 $2) -f "$PRJ_ROOT/ci/docker/uberbase.Dockerfile" .
}

push () {
    docker push $(tag $1 $2)
}

if [[ $(uname -m) == "aarch64" ]]; then
  build-and-push amazon-linux-2       linux/arm64
else
  build all linux/amd64

  build alpine-linux-3.15    linux/amd64
  build alpine-linux-3.19    linux/amd64
  build amazon-linux-2       linux/amd64
  build amazon-linux-2018.03 linux/amd64
  build amazon-linux-2023    linux/amd64
  build arch-linux           linux/amd64
  build ubuntu-linux-18.04   linux/amd64
  build ubuntu-linux-22.04   linux/amd64

  push  alpine-linux-3.15    linux/amd64
  push  alpine-linux-3.19    linux/amd64
  push  amazon-linux-2       linux/amd64
  push  amazon-linux-2018.03 linux/amd64
  push  amazon-linux-2023    linux/amd64
  push  arch-linux           linux/amd64
  push  ubuntu-linux-18.04   linux/amd64
  push  ubuntu-linux-22.04   linux/amd64
fi
