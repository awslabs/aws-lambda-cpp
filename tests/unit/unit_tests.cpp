#include <aws/lambda-runtime/runtime.h>
#include <aws/lambda-runtime/version.h>
#include <aws/lambda-runtime/outcome.h>
#include <aws/http/response.h>
#include "gtest/gtest.h"

using namespace aws::lambda_runtime;
using namespace aws::http;

// --- invocation_response tests ---

TEST(InvocationResponseTest, success_response_has_correct_payload_and_content_type)
{
    auto resp = invocation_response::success("hello world", "text/plain");
    EXPECT_TRUE(resp.is_success());
    EXPECT_EQ("hello world", resp.get_payload());
    EXPECT_EQ("text/plain", resp.get_content_type());
}

TEST(InvocationResponseTest, success_response_with_json)
{
    auto resp = invocation_response::success(R"({"key":"value"})", "application/json");
    EXPECT_TRUE(resp.is_success());
    EXPECT_EQ(R"({"key":"value"})", resp.get_payload());
    EXPECT_EQ("application/json", resp.get_content_type());
}

TEST(InvocationResponseTest, success_response_with_empty_payload)
{
    auto resp = invocation_response::success("", "application/json");
    EXPECT_TRUE(resp.is_success());
    EXPECT_EQ("", resp.get_payload());
}

TEST(InvocationResponseTest, failure_response_is_not_success)
{
    auto resp = invocation_response::failure("something broke", "RuntimeError");
    EXPECT_FALSE(resp.is_success());
    EXPECT_EQ("application/json", resp.get_content_type());
}

TEST(InvocationResponseTest, failure_response_contains_error_message)
{
    auto resp = invocation_response::failure("something broke", "RuntimeError");
    auto const& payload = resp.get_payload();
    EXPECT_NE(std::string::npos, payload.find("something broke"));
    EXPECT_NE(std::string::npos, payload.find("RuntimeError"));
}

TEST(InvocationResponseTest, failure_response_json_escapes_quotes)
{
    auto resp = invocation_response::failure(R"(error with "quotes")", "TestError");
    auto const& payload = resp.get_payload();
    EXPECT_NE(std::string::npos, payload.find(R"(error with \"quotes\")"));
    EXPECT_EQ(std::string::npos, payload.find(R"(error with "quotes")"));
}

TEST(InvocationResponseTest, failure_response_json_escapes_backslash)
{
    auto resp = invocation_response::failure(R"(path\to\file)", "TestError");
    auto const& payload = resp.get_payload();
    EXPECT_NE(std::string::npos, payload.find(R"(path\\to\\file)"));
}

TEST(InvocationResponseTest, failure_response_json_escapes_newlines)
{
    auto resp = invocation_response::failure("line1\nline2\r\n", "TestError");
    auto const& payload = resp.get_payload();
    EXPECT_NE(std::string::npos, payload.find(R"(line1\nline2\r\n)"));
}

TEST(InvocationResponseTest, failure_response_json_escapes_tabs)
{
    auto resp = invocation_response::failure("col1\tcol2", "TestError");
    auto const& payload = resp.get_payload();
    EXPECT_NE(std::string::npos, payload.find(R"(col1\tcol2)"));
}

TEST(InvocationResponseTest, failure_response_json_escapes_control_characters)
{
    std::string msg = "null\x00 byte";
    msg.push_back('\x01');
    auto resp = invocation_response::failure(msg, "TestError");
    auto const& payload = resp.get_payload();
    EXPECT_NE(std::string::npos, payload.find("\\u0001"));
}

TEST(InvocationResponseTest, success_response_with_binary_content_type)
{
    std::string binary_data(256, '\0');
    for (int i = 0; i < 256; ++i) {
        binary_data[static_cast<size_t>(i)] = static_cast<char>(i);
    }
    auto resp = invocation_response::success(binary_data, "application/octet-stream");
    EXPECT_TRUE(resp.is_success());
    EXPECT_EQ(256u, resp.get_payload().size());
}

TEST(InvocationResponseTest, constructor_based_failure)
{
    auto resp = invocation_response(R"({"custom":"error"})", "application/json", false);
    EXPECT_FALSE(resp.is_success());
    EXPECT_EQ(R"({"custom":"error"})", resp.get_payload());
}

// --- runtime_response tests ---

TEST(RuntimeResponseTest, constructor_sets_all_fields)
{
    runtime_response resp("payload data", "application/json", "xray-trace-123");
    EXPECT_EQ("payload data", resp.get_payload());
    EXPECT_EQ("application/json", resp.get_content_type());
    EXPECT_EQ("xray-trace-123", resp.get_xray_response());
}

TEST(RuntimeResponseTest, constructor_with_empty_fields)
{
    runtime_response resp("", "", "");
    EXPECT_EQ("", resp.get_payload());
    EXPECT_EQ("", resp.get_content_type());
    EXPECT_EQ("", resp.get_xray_response());
}

TEST(RuntimeResponseTest, constructor_with_empty_xray)
{
    runtime_response resp("error body", "application/json", "");
    EXPECT_EQ("error body", resp.get_payload());
    EXPECT_EQ("application/json", resp.get_content_type());
    EXPECT_EQ("", resp.get_xray_response());
}

TEST(RuntimeResponseTest, large_payload)
{
    std::string large(10000, 'x');
    runtime_response resp(large, "text/plain", "");
    EXPECT_EQ(10000u, resp.get_payload().size());
    EXPECT_EQ(large, resp.get_payload());
}

TEST(RuntimeResponseTest, can_be_used_for_init_error)
{
    runtime_response init_err(
        R"({"errorMessage":"module not found","errorType":"ImportError"})", "application/json", "xray-cause-data");
    EXPECT_EQ("application/json", init_err.get_content_type());
    EXPECT_NE(std::string::npos, init_err.get_payload().find("module not found"));
    EXPECT_EQ("xray-cause-data", init_err.get_xray_response());
}

// --- invocation_response inheritance tests ---

TEST(InvocationResponseInheritanceTest, is_a_runtime_response)
{
    invocation_response resp("payload", "text/plain", true, "xray");
    runtime_response const& base = resp;
    EXPECT_EQ("payload", base.get_payload());
    EXPECT_EQ("text/plain", base.get_content_type());
    EXPECT_EQ("xray", base.get_xray_response());
}

TEST(InvocationResponseInheritanceTest, three_arg_constructor_has_empty_xray)
{
    invocation_response resp("data", "text/html", true);
    EXPECT_EQ("data", resp.get_payload());
    EXPECT_EQ("text/html", resp.get_content_type());
    EXPECT_EQ("", resp.get_xray_response());
    EXPECT_TRUE(resp.is_success());
}

TEST(InvocationResponseInheritanceTest, four_arg_constructor_preserves_xray)
{
    invocation_response resp("err", "application/json", false, "xray-data");
    EXPECT_EQ("err", resp.get_payload());
    EXPECT_EQ("application/json", resp.get_content_type());
    EXPECT_EQ("xray-data", resp.get_xray_response());
    EXPECT_FALSE(resp.is_success());
}

TEST(InvocationResponseInheritanceTest, failure_with_xray_response)
{
    auto resp = invocation_response::failure("err msg", "ErrType", "xray-cause");
    EXPECT_FALSE(resp.is_success());
    EXPECT_EQ("application/json", resp.get_content_type());
    EXPECT_EQ("xray-cause", resp.get_xray_response());
    EXPECT_NE(std::string::npos, resp.get_payload().find("err msg"));
}

TEST(InvocationResponseInheritanceTest, success_has_empty_xray)
{
    auto resp = invocation_response::success("ok", "text/plain");
    EXPECT_TRUE(resp.is_success());
    EXPECT_EQ("", resp.get_xray_response());
}

TEST(InvocationResponseInheritanceTest, can_pass_as_runtime_response_const_ref)
{
    invocation_response resp("body", "application/json", false, "xray-123");
    auto check = [](runtime_response const& r) {
        EXPECT_EQ("body", r.get_payload());
        EXPECT_EQ("application/json", r.get_content_type());
        EXPECT_EQ("xray-123", r.get_xray_response());
    };
    check(resp);
}

// --- http::response tests ---

TEST(HttpResponseTest, add_and_retrieve_header)
{
    response resp;
    resp.add_header("Content-Type", "application/json");
    EXPECT_TRUE(resp.has_header("content-type"));
    auto header = resp.get_header("content-type");
    EXPECT_TRUE(header.is_success());
    EXPECT_EQ("application/json", header.get_result());
}

TEST(HttpResponseTest, headers_are_lowercased)
{
    response resp;
    resp.add_header("X-Custom-Header", "some-value");
    EXPECT_TRUE(resp.has_header("x-custom-header"));
    EXPECT_FALSE(resp.has_header("X-Custom-Header"));
}

TEST(HttpResponseTest, has_header_returns_false_for_missing)
{
    response resp;
    resp.add_header("Content-Type", "text/plain");
    EXPECT_FALSE(resp.has_header("x-missing"));
}

TEST(HttpResponseTest, append_body_accumulates)
{
    response resp;
    resp.append_body("hello", 5);
    resp.append_body(" world", 6);
    EXPECT_EQ("hello world", resp.get_body());
}

TEST(HttpResponseTest, append_body_empty)
{
    response resp;
    EXPECT_EQ("", resp.get_body());
}

TEST(HttpResponseTest, set_response_code)
{
    response resp;
    resp.set_response_code(response_code::OK);
    EXPECT_EQ(response_code::OK, resp.get_response_code());
}

TEST(HttpResponseTest, multiple_headers)
{
    response resp;
    resp.add_header("lambda-runtime-aws-request-id", "req-123");
    resp.add_header("lambda-runtime-trace-id", "trace-456");
    resp.add_header("lambda-runtime-deadline-ms", "1234567890");
    EXPECT_EQ("req-123", resp.get_header("lambda-runtime-aws-request-id").get_result());
    EXPECT_EQ("trace-456", resp.get_header("lambda-runtime-trace-id").get_result());
    EXPECT_EQ("1234567890", resp.get_header("lambda-runtime-deadline-ms").get_result());
}

// --- outcome tests ---

TEST(OutcomeTest, success_outcome)
{
    outcome<std::string, int> o(std::string("result"));
    EXPECT_TRUE(o.is_success());
    EXPECT_EQ("result", o.get_result());
}

TEST(OutcomeTest, failure_outcome)
{
    outcome<std::string, int> o(42);
    EXPECT_FALSE(o.is_success());
    EXPECT_EQ(42, o.get_failure());
}

TEST(OutcomeTest, move_success)
{
    outcome<std::string, int> o1(std::string("moved"));
    outcome<std::string, int> o2(std::move(o1));
    EXPECT_TRUE(o2.is_success());
    EXPECT_EQ("moved", o2.get_result());
}

TEST(OutcomeTest, move_failure)
{
    outcome<std::string, int> o1(99);
    outcome<std::string, int> o2(std::move(o1));
    EXPECT_FALSE(o2.is_success());
    EXPECT_EQ(99, o2.get_failure());
}

TEST(OutcomeTest, with_response_code)
{
    using test_outcome = outcome<no_result, response_code>;
    test_outcome success(no_result{});
    EXPECT_TRUE(success.is_success());

    test_outcome failure(response_code::INTERNAL_SERVER_ERROR);
    EXPECT_FALSE(failure.is_success());
    EXPECT_EQ(response_code::INTERNAL_SERVER_ERROR, failure.get_failure());
}

// --- invocation_request tests ---

TEST(InvocationRequestTest, get_time_remaining_future_deadline)
{
    invocation_request req;
    req.deadline = std::chrono::system_clock::now() + std::chrono::seconds(30);
    auto remaining = req.get_time_remaining();
    EXPECT_GT(remaining.count(), 29000);
    EXPECT_LE(remaining.count(), 30000);
}

TEST(InvocationRequestTest, get_time_remaining_past_deadline)
{
    invocation_request req;
    req.deadline = std::chrono::system_clock::now() - std::chrono::seconds(5);
    auto remaining = req.get_time_remaining();
    EXPECT_LT(remaining.count(), 0);
}

TEST(InvocationRequestTest, default_fields_are_empty)
{
    invocation_request req;
    EXPECT_TRUE(req.payload.empty());
    EXPECT_TRUE(req.request_id.empty());
    EXPECT_TRUE(req.xray_trace_id.empty());
    EXPECT_TRUE(req.client_context.empty());
    EXPECT_TRUE(req.cognito_identity.empty());
    EXPECT_TRUE(req.function_arn.empty());
    EXPECT_TRUE(req.tenant_id.empty());
}

// --- version tests (no AWS SDK needed) ---

TEST(VersionTest, version_string_not_empty)
{
    EXPECT_NE(nullptr, get_version());
    EXPECT_GT(strlen(get_version()), 0u);
}

TEST(VersionTest, version_format)
{
    std::string v = get_version();
    int dots = 0;
    for (char c : v) {
        if (c == '.')
            dots++;
    }
    EXPECT_EQ(2, dots);
}
