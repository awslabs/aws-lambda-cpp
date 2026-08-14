#!/bin/bash
# Post-release OCI smoke test.
#
# Downloads the published static library from the (draft) release, verifies its
# checksum against the published SHA256SUMS, links a dummy handler against it,
# and invokes it through the Lambda Runtime Interface Emulator baked into
# public.ecr.aws/lambda/provided:al2023 — i.e. the exact happy path a downstream
# consumer of the artifact would follow.
#
# Requires: gh (with GH_TOKEN), docker, curl. Runs on stock GitHub-hosted
# runners; needs no AWS credentials or CodeArtifact access.
#
# Usage: run-oci-smoke.sh <arch> <tag>
set -euo pipefail

ARCH=${1:?usage: run-oci-smoke.sh <arch> <tag>}
TAG=${2:?usage: run-oci-smoke.sh <arch> <tag>}
REPO=${GITHUB_REPOSITORY:-awslabs/aws-lambda-cpp}
PORT=9000

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
REPO_ROOT=$(cd "$SCRIPT_DIR/../.." && pwd)

LIB="libaws-lambda-runtime-${ARCH}.a"
IMAGE="lambda-cpp-oci-smoke:${ARCH}"
CONTAINER="lambda-cpp-oci-smoke-${ARCH}"

WORKDIR=$(mktemp -d)
trap 'docker rm -f "$CONTAINER" >/dev/null 2>&1 || true; rm -rf "$WORKDIR"' EXIT

echo "== Downloading $LIB + SHA256SUMS from release '$TAG' =="
gh release download "$TAG" --repo "$REPO" \
  -p "$LIB" -p SHA256SUMS --dir "$WORKDIR" --clobber

echo "== Verifying checksum =="
( cd "$WORKDIR" && grep -F "$LIB" SHA256SUMS | sha256sum -c - )

echo "== Assembling build context =="
CTX="$WORKDIR/ctx"
mkdir -p "$CTX"
cp -r "$REPO_ROOT/include" "$CTX/include"
cp "$REPO_ROOT/ci/integ/oci/main.cpp" "$CTX/main.cpp"
cp "$WORKDIR/$LIB" "$CTX/runtime.a"

echo "== Building OCI image =="
docker build \
  -f "$REPO_ROOT/ci/integ/docker/Dockerfile.oci-smoke" \
  -t "$IMAGE" "$CTX"

echo "== Starting container (RIE) =="
docker run -d --name "$CONTAINER" -p "$PORT:8080" "$IMAGE" >/dev/null

INVOKE_URL="http://localhost:${PORT}/2015-03-31/functions/function/invocations"

echo "== Waiting for RIE to become ready =="
ready=false
for _ in $(seq 1 30); do
  if curl -sf -XPOST "$INVOKE_URL" -d '{}' >/dev/null 2>&1; then
    ready=true
    break
  fi
  sleep 1
done
if [ "$ready" != true ]; then
  echo "::error::RIE did not become ready in time"
  docker logs "$CONTAINER" || true
  exit 1
fi

echo "== Invoking via RIE =="
RESPONSE=$(curl -s -XPOST "$INVOKE_URL" -d '{"answer":42}')
echo "Response: $RESPONSE"

EXPECTED='"payload_length":13'
if ! echo "$RESPONSE" | grep -qF "$EXPECTED"; then
  echo "::error::Smoke test assertion failed for $ARCH. Expected response to contain: $EXPECTED"
  docker logs "$CONTAINER" || true
  exit 1
fi

echo "Smoke test passed for $ARCH"
