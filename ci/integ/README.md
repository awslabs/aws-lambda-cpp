# Integration Tests

This directory contains scripts for building, deploying, invoking, and asserting Lambda functions.
CI runs these automatically via GitHub Actions, but you can also run them locally.

## Prerequisites

- CMake and Ninja
- AWS CLI configured with credentials that can create/invoke/delete Lambda functions
- An IAM execution role for Lambda (with basic execution permissions)
- OS-specific compiler (gcc/g++ on AL2023, clang on Ubuntu/Arch)

## Environment Variables

| Variable | Description |
|----------|-------------|
| `AWS_REGION` | AWS region (e.g. `us-east-1`) |
| `LAMBDA_EXECUTION_ROLE_ARN` | ARN of the Lambda execution role |

## Running Zip Integration Tests Locally

To run the full zip integration test (build, deploy, invoke, assert, cleanup) in one shot:

```bash
./ci/integ/run-zip-integ.sh [OS] [HANDLER] [PAYLOAD] [ASSERTION] [ARCH]
```

Defaults: `al2023`, `echo_success`, `'{"barbaz":"Hello, Lambda!"}'`, `snapshot`, `x86_64`.

Example:
```bash
export AWS_REGION="us-east-1"
export LAMBDA_EXECUTION_ROLE_ARN="arn:aws:iam::123456789012:role/your-lambda-role"

./ci/integ/run-zip-integ.sh al2023
```
