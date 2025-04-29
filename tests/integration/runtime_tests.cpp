#include <aws/lambda/model/Architecture.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/utils/Array.h>
#include <aws/core/utils/Outcome.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/core/utils/memory/stl/AWSStringStream.h>
#include <aws/core/platform/Environment.h>
#include <aws/iam/IAMClient.h>
#include <aws/iam/model/GetRoleRequest.h>
#include <aws/lambda/LambdaClient.h>
#include <aws/lambda/model/DeleteFunctionRequest.h>
#include <aws/lambda/model/GetFunctionRequest.h>
#include <aws/lambda/model/CreateFunctionRequest.h>
#include <aws/lambda/model/InvokeRequest.h>
#include <aws/core/utils/base64/Base64.h>
#include "../gtest/gtest.h"
#include <aws/lambda/model/LogType.h>
#include <cstdio>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <string>

extern std::string aws_prefix;

namespace {

using namespace Aws::Lambda;

constexpr auto ZIP_FILE_PATH = "resources/lambda-test-fun.zip";
constexpr auto REQUEST_TIMEOUT = 15 * 1000;

struct LambdaRuntimeTest : public ::testing::Test {
    LambdaClient m_lambda_client;
    Aws::IAM::IAMClient m_iam_client;
    static Aws::Client::ClientConfiguration create_iam_config()
    {
        Aws::Client::ClientConfiguration config;
        config.requestTimeoutMs = REQUEST_TIMEOUT;
        config.region = Aws::Region::US_EAST_1;
        return config;
    }

    static Aws::Client::ClientConfiguration create_lambda_config()
    {
        Aws::Client::ClientConfiguration config;
        config.requestTimeoutMs = REQUEST_TIMEOUT;
        config.region = Aws::Environment::GetEnv("AWS_REGION");
        if (config.region.empty()) {
            throw std::invalid_argument("environment variable AWS_REGION not set");
        }
        return config;
    }

    static Aws::String build_resource_name(Aws::String const& name)
    {
        return aws_prefix.c_str() + name; // NOLINT
    }

    LambdaRuntimeTest() : m_lambda_client(create_lambda_config()), m_iam_client(create_iam_config()) {}

    ~LambdaRuntimeTest() override
    {
        // clean up in case we exited one test abnormally
        delete_function(build_resource_name("echo_success"), false /*assert*/);
        delete_function(build_resource_name("echo_failure"), false /*assert*/);
        delete_function(build_resource_name("binary_response"), false /*assert*/);
        delete_function(build_resource_name("crash_backtrace"), false /*assert*/);
    }

    Aws::String get_role_arn(Aws::String const& role_name)
    {
        using namespace Aws::IAM;
        using namespace Aws::IAM::Model;
        GetRoleRequest request;
        request.WithRoleName(role_name);
        auto outcome = m_iam_client.GetRole(request);
        EXPECT_TRUE(outcome.IsSuccess());
        if (outcome.IsSuccess()) {
            return outcome.GetResult().GetRole().GetArn();
        }
        return {};
    }

    void create_function(Aws::String const& function_name, Aws::String const& handler_name)
    {
        Model::CreateFunctionRequest create_function_request;
        create_function_request.SetHandler(handler_name);
        create_function_request.SetFunctionName(function_name);
        // I ran into eventual-consistency issues when creating the role dynamically as part of the test.
        auto exec_role = Aws::Environment::GetEnv("LAMBDA_TEST_ROLE");
        if (exec_role.empty()) {
            exec_role = "integration-tests";
        }
        create_function_request.SetRole(get_role_arn(exec_role));

        struct stat s;
        auto rc = stat(ZIP_FILE_PATH, &s);
        ASSERT_EQ(rc, 0) << std::string("file does not exist: ") + ZIP_FILE_PATH;
        Aws::Utils::CryptoBuffer zip_file_bytes(s.st_size);
        auto* zip_file = fopen(ZIP_FILE_PATH, "r");
        fread(zip_file_bytes.GetUnderlyingData(), sizeof(unsigned char), s.st_size, zip_file);
        fclose(zip_file);

        Model::FunctionCode funcode;
        funcode.SetZipFile(std::move(zip_file_bytes));
        create_function_request.SetCode(std::move(funcode));
        create_function_request.SetRuntime(Aws::Lambda::Model::Runtime::provided_al2);

        std::vector<Aws::Lambda::Model::Architecture> lambda_architectures = {Aws::Lambda::Model::Architecture::x86_64};
#ifdef __aarch64__
        lambda_architectures[0] = Aws::Lambda::Model::Architecture::arm64;
#endif
        create_function_request.SetArchitectures(lambda_architectures);

        auto outcome = m_lambda_client.CreateFunction(create_function_request);
        ASSERT_TRUE(outcome.IsSuccess()) << "Failed to create function " << function_name;

        // work around Lambda function pending creation state
        sleep(5);
    }

    void delete_function(Aws::String const& function_name, bool assert = true)
    {
        Model::DeleteFunctionRequest delete_function_request;
        delete_function_request.SetFunctionName(function_name);
        auto outcome = m_lambda_client.DeleteFunction(delete_function_request);
        if (assert) {
            ASSERT_TRUE(outcome.IsSuccess()) << "Failed to delete function " << function_name;
        }
    }
};

TEST_F(LambdaRuntimeTest, echo_success)
{
    Aws::String const funcname = build_resource_name("echo_success");
    constexpr auto payload_content = "Hello, Lambda!";
    create_function(funcname, "echo_success" /*handler_name*/);
    Model::InvokeRequest invoke_request;
    invoke_request.SetFunctionName(funcname);
    invoke_request.SetInvocationType(Model::InvocationType::RequestResponse);
    invoke_request.SetContentType("application/json");

    std::shared_ptr<Aws::IOStream> payload = Aws::MakeShared<Aws::StringStream>("FunctionTest");
    Aws::Utils::Json::JsonValue json_payload;
    json_payload.WithString("barbaz", payload_content);
    *payload << json_payload.View().WriteCompact();
    invoke_request.SetBody(payload);

    Model::InvokeOutcome invoke_outcome = m_lambda_client.Invoke(invoke_request);
    EXPECT_TRUE(invoke_outcome.IsSuccess());
    Aws::StringStream output;
    if (!invoke_outcome.IsSuccess()) {
        delete_function(funcname);
        return;
    }
    EXPECT_EQ(200, invoke_outcome.GetResult().GetStatusCode());
    EXPECT_TRUE(invoke_outcome.GetResult().GetFunctionError().empty());
    auto const json_response = Aws::Utils::Json::JsonValue(invoke_outcome.GetResult().GetPayload());
    EXPECT_TRUE(json_response.WasParseSuccessful());
    EXPECT_STREQ(payload_content, json_response.View().GetString("barbaz").c_str());
    delete_function(funcname);
}

TEST_F(LambdaRuntimeTest, echo_unicode)
{
    Aws::String const funcname = build_resource_name("echo_success"); // re-use the echo method but with Unicode input
    constexpr auto payload_content = "画像は1000語の価値がある";
    create_function(funcname, "echo_success" /*handler_name*/);
    Model::InvokeRequest invoke_request;
    invoke_request.SetFunctionName(funcname);
    invoke_request.SetInvocationType(Model::InvocationType::RequestResponse);
    invoke_request.SetContentType("application/json");

    std::shared_ptr<Aws::IOStream> payload = Aws::MakeShared<Aws::StringStream>("FunctionTest");
    Aws::Utils::Json::JsonValue json_payload;
    json_payload.WithString("UnicodeText", payload_content);
    *payload << json_payload.View().WriteCompact();
    invoke_request.SetBody(payload);

    Model::InvokeOutcome invoke_outcome = m_lambda_client.Invoke(invoke_request);
    EXPECT_TRUE(invoke_outcome.IsSuccess());
    Aws::StringStream output;
    if (!invoke_outcome.IsSuccess()) {
        delete_function(funcname);
        return;
    }
    EXPECT_EQ(200, invoke_outcome.GetResult().GetStatusCode());
    EXPECT_TRUE(invoke_outcome.GetResult().GetFunctionError().empty());
    auto const json_response = Aws::Utils::Json::JsonValue(invoke_outcome.GetResult().GetPayload());
    EXPECT_TRUE(json_response.WasParseSuccessful());
    EXPECT_STREQ(payload_content, json_response.View().GetString("UnicodeText").c_str());
    delete_function(funcname);
}

TEST_F(LambdaRuntimeTest, echo_failure)
{
    Aws::String const funcname = build_resource_name("echo_failure");
    create_function(funcname, "echo_failure" /*handler_name*/);
    Model::InvokeRequest invoke_request;
    invoke_request.SetFunctionName(funcname);
    invoke_request.SetInvocationType(Model::InvocationType::RequestResponse);

    Model::InvokeOutcome invoke_outcome = m_lambda_client.Invoke(invoke_request);
    EXPECT_TRUE(invoke_outcome.IsSuccess());
    EXPECT_EQ(200, invoke_outcome.GetResult().GetStatusCode());
    EXPECT_STREQ("Unhandled", invoke_outcome.GetResult().GetFunctionError().c_str());
    delete_function(funcname);
}

TEST_F(LambdaRuntimeTest, binary_response)
{
    Aws::String const funcname = build_resource_name("binary_response");
    unsigned long constexpr expected_length = 1451;
    create_function(funcname, "binary_response" /*handler_name*/);
    Model::InvokeRequest invoke_request;
    invoke_request.SetFunctionName(funcname);
    invoke_request.SetInvocationType(Model::InvocationType::RequestResponse);

    Model::InvokeOutcome invoke_outcome = m_lambda_client.Invoke(invoke_request);
    EXPECT_TRUE(invoke_outcome.IsSuccess());
    EXPECT_EQ(200, invoke_outcome.GetResult().GetStatusCode());
    EXPECT_TRUE(invoke_outcome.GetResult().GetFunctionError().empty());
    EXPECT_EQ(expected_length, invoke_outcome.GetResult().GetPayload().tellp());
    delete_function(funcname);
}

TEST_F(LambdaRuntimeTest, crash)
{
    Aws::String const funcname = build_resource_name("crash_backtrace");
    create_function(funcname, "crash_backtrace" /*handler_name*/);
    Model::InvokeRequest invoke_request;
    invoke_request.SetFunctionName(funcname);
    invoke_request.SetInvocationType(Model::InvocationType::RequestResponse);
    invoke_request.SetLogType(Model::LogType::Tail);

    Model::InvokeOutcome invoke_outcome = m_lambda_client.Invoke(invoke_request);
    EXPECT_TRUE(invoke_outcome.IsSuccess());
    EXPECT_EQ(200, invoke_outcome.GetResult().GetStatusCode());
    EXPECT_STREQ("Unhandled", invoke_outcome.GetResult().GetFunctionError().c_str());
    Aws::Utils::Base64::Base64 base64;
    auto decoded = base64.Decode(invoke_outcome.GetResult().GetLogResult());
    std::string tail_logs(reinterpret_cast<char const*>(decoded.GetUnderlyingData()), decoded.GetLength());
    EXPECT_NE(tail_logs.find("Stack trace (most recent call last):"), std::string::npos);
    delete_function(funcname);
}

} // namespace
