#!/bin/bash
set -euo pipefail

FUNCTION_NAME=$1
PAYLOAD=${2:-}
ASSERTION=$3
TEST_NAME=$4
SNAPSHOTS_DIR="tests/snapshots"

TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

if [ -n "$PAYLOAD" ]; then
  echo "$PAYLOAD" > "$TMPDIR/payload.json"
  PAYLOAD_ARG="--payload fileb://$TMPDIR/payload.json"
else
  PAYLOAD_ARG=""
fi

aws lambda invoke \
  --function-name "$FUNCTION_NAME" \
  --invocation-type RequestResponse \
  --log-type Tail \
  --cli-read-timeout 30 \
  $PAYLOAD_ARG \
  "$TMPDIR/response_payload" > "$TMPDIR/invoke_result.json"

echo "Status code: $(jq '.StatusCode' "$TMPDIR/invoke_result.json")"
echo "Function error: $(jq -r '.FunctionError // empty' "$TMPDIR/invoke_result.json")"

case "$ASSERTION" in
  snapshot)
    EXPECTED=$(cat "$SNAPSHOTS_DIR/${TEST_NAME}.expected")
    ACTUAL=$(cat "$TMPDIR/response_payload")
    if [ "$EXPECTED" != "$ACTUAL" ]; then
      echo "::error::Snapshot mismatch for $TEST_NAME"
      echo "Expected: $EXPECTED"
      echo "Actual:   $ACTUAL"
      exit 1
    fi
    echo "Snapshot matched"
    ;;
  length)
    EXPECTED_LENGTH=$(cat "$SNAPSHOTS_DIR/${TEST_NAME}.expected_length")
    ACTUAL_LENGTH=$(wc -c < "$TMPDIR/response_payload")
    if [ "$EXPECTED_LENGTH" != "$ACTUAL_LENGTH" ]; then
      echo "::error::Length mismatch for $TEST_NAME"
      echo "Expected: $EXPECTED_LENGTH bytes"
      echo "Actual:   $ACTUAL_LENGTH bytes"
      exit 1
    fi
    echo "Length matched: $ACTUAL_LENGTH bytes"
    ;;
  contains)
    EXPECTED_SUBSTR=$(cat "$SNAPSHOTS_DIR/${TEST_NAME}.expected_contains")
    LOG_TAIL=$(jq -r '.LogResult' "$TMPDIR/invoke_result.json" | base64 -d)
    FUNCTION_ERROR=$(jq -r '.FunctionError' "$TMPDIR/invoke_result.json")
    if [ "$FUNCTION_ERROR" != "Unhandled" ]; then
      echo "::error::Expected FunctionError=Unhandled, got: $FUNCTION_ERROR"
      exit 1
    fi
    if echo "$LOG_TAIL" | grep -qF "$EXPECTED_SUBSTR"; then
      echo "Log contains expected string"
    else
      echo "::error::Log does not contain expected string for $TEST_NAME"
      echo "Expected substring: $EXPECTED_SUBSTR"
      echo "Actual log tail: $LOG_TAIL"
      exit 1
    fi
    ;;
  *)
    echo "Unknown assertion type: $ASSERTION" && exit 1
    ;;
esac
