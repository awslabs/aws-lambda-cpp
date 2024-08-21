#include <aws/lambda-runtime/runtime.h>

using namespace aws::lambda_runtime;

static invocation_response my_handler(invocation_request const& req)
{
    if (req.payload.length() > 42) {
        return invocation_response::failure("error message here"/*error_message*/,
                                            "error type here" /*error_type*/);
    }

    return invocation_response::success("{\"message:\":\"I fail if body length is bigger than 42!\"}" /*payload*/,
                                        "application/json" /*MIME type*/);
}

int main()
{
    run_handler(my_handler);
    return 0;
}
