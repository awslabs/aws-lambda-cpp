#!/bin/bash
set -euo pipefail

OS=${1:-al2023}
HANDLER=${2:-echo_success}
PAYLOAD=${3:-'{"barbaz":"Hello, Lambda!"}'}
ASSERTION=${4:-snapshot}
ARCH=${5:-x86_64}
MODE=${6:-default}

: "${AWS_REGION:?Set AWS_REGION}"
: "${LAMBDA_EXECUTION_ROLE_ARN:?Set LAMBDA_EXECUTION_ROLE_ARN}"

FUNCTION_NAME="lambda-cpp-integ-local-$(date +%s)"

cleanup() {
  echo "Cleaning up $FUNCTION_NAME..."
  aws lambda delete-function --function-name "$FUNCTION_NAME" 2>/dev/null || true
}
trap cleanup EXIT

echo "==> Building and packaging zip ($OS, mode=$MODE)..."
./ci/integ/package-zip.sh "$OS" "$MODE"

echo "==> Deploying Lambda function ($FUNCTION_NAME)..."
aws lambda create-function \
  --function-name "$FUNCTION_NAME" \
  --runtime provided.al2023 \
  --handler "$HANDLER" \
  --architectures "$ARCH" \
  --role "$LAMBDA_EXECUTION_ROLE_ARN" \
  --timeout 30 \
  --zip-file fileb://build/tests/resources/lambda-test-fun.zip \
  --environment "Variables={HANDLER=$HANDLER}"

timeout 60 aws lambda wait function-active-v2 --function-name "$FUNCTION_NAME"

echo "==> Invoking and asserting ($ASSERTION: $HANDLER)..."
./ci/integ/invoke-and-assert.sh "$FUNCTION_NAME" "$PAYLOAD" "$ASSERTION" "$HANDLER"

echo "==> Passed!"
