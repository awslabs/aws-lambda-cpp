# PR #216 Feedback Intake

## Summary of Reviewer Feedback (bmoffatt)

### 1. Missing .zip packaging coverage (CRITICAL)
The new workflow only tests OCI (Docker image) deployments. The old CodeBuild CI also tested the **`packaging/packager` script** which produces `.zip` artifacts deployed with `runtime: provided.al2023`. This is a key feature of the project — it allows users to ship Lambda functions without Docker.

**Reviewer's ask:** Add a `.zip` deployment path that runs `ninja install aws-lambda-package-lambda-test-fun`, uploads the zip, creates a function with `provided.al2023` runtime, invokes, and asserts — similar matrix to the existing unit-test job.

### 2. Keep OCI tests too
Reviewer explicitly confirmed the OCI tests are valuable and should stay alongside the new zip tests.

### 3. Losing the ability to run integration tests locally
The old approach allowed developers to run integration tests locally (the README documents this). The new approach can only run in CI. Reviewer would like to keep some way to run locally, but acknowledges the AWS SDK C++ build time is a pain — open to alternatives that don't explode CI runtimes.

---

## Questions Before Implementing

1. **Zip test scope:** Should the zip-based integration test cover the same 5-OS matrix (al2023, al2023-arm, ubuntu, alpine, arch), or is a smaller subset acceptable? The packager script relies on glibc introspection which won't work on Alpine/musl — should we skip Alpine for zip tests?

yes

2. **Lambda runtime for zip:** The old tests used `provided.al2` (which is EOL). Should the new zip tests target `provided.al2023`?

yes

3. **Local integration test story:** What's the acceptable bar here? Options:
   - (a) Keep the shell scripts usable standalone (document env vars needed: AWS creds, ECR repo, etc.) so someone can run them locally with their own AWS account
   - (b) Restore a CMake-integrated test (needs AWS SDK C++ — slow to build, but could be cached)
   - (c) Just document "run the GHA workflow from a fork" and call it good enough
   
   Which approach do you prefer? Or is (a) sufficient since the shell scripts in `ci/integ/` are already standalone?

yes, a is good as long as it's clearly documented on how to run those test locally

4. **`_HANDLER` vs `HANDLER` env var:** For the zip path, Lambda sets `_HANDLER` automatically from the function configuration's handler field. The OCI path uses a custom `HANDLER` env var because there's no handler field for image-based functions. Should the test binary support both (check `_HANDLER` first, fall back to `HANDLER`) so it works in either deployment mode without code changes?

yes