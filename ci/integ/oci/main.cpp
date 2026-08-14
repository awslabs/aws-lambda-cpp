#include <aws/lambda-runtime/runtime.h>

using namespace aws::lambda_runtime;

// Dummy handler used by the post-release OCI smoke test. It echoes the incoming
// payload back and reports its length, which the smoke test asserts on.
static invocation_response my_handler(invocation_request const& req)
{
    return invocation_response::success(
        R"({"message":"hello from aws-lambda-cpp","echo":)" + (req.payload.empty() ? std::string("null") : req.payload) +
            R"(,"payload_length":)" + std::to_string(req.payload.length()) + "}",
        "application/json");
}

int main()
{
    run_handler(my_handler);
    return 0;
}
