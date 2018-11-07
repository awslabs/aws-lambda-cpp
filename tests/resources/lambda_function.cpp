#include <aws/lambda-runtime/runtime.h>

#include <unordered_map>

using namespace aws::lambda_runtime;

invocation_response echo_success(invocation_request const& request)
{
    return invocation_response::success(request.payload, "application/json");
}

invocation_response echo_failure(invocation_request const&)
{
    return invocation_response::failure("Test error message", "TestErrorType");
}

int main(int argc, char* argv[])
{
    using handler_fn = std::function<aws::lambda_runtime::invocation_response(aws::lambda_runtime::invocation_request const&)>;
    std::unordered_map<std::string, handler_fn> handlers;
    handlers.emplace("echo_success", echo_success);
    handlers.emplace("echo_failure", echo_failure);

    if (argc < 2) {
        aws::logging::log_error("lambda_test", "missing handler argument. Exiting.");
        return -1;
    }

    auto it = handlers.find(argv[1]);
    if (it == handlers.end()) {
        aws::logging::log_error("lambda_test", "handler %s not found. Exiting.", argv[1]);
        return -2;
    }

    run_handler(it->second);
    return 0;
}
