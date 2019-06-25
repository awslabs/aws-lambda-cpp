#include <aws/core/client/ClientConfiguration.h>
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
#include <aws/lambda/model/DeleteFunctionRequest.h>
#include <aws/lambda/model/InvokeRequest.h>
#include "gtest/gtest.h"

extern std::string aws_prefix;

namespace {

using namespace Aws::Lambda;

const char S3BUCKET[] = "aws-lambda-cpp-tests";
const char S3KEY[] = "lambda-test-fun.zip";

struct LambdaRuntimeTest : public ::testing::Test {
    LambdaClient m_lambda_client;
    Aws::IAM::IAMClient m_iam_client;
    static Aws::Client::ClientConfiguration create_iam_config()
    {
        Aws::Client::ClientConfiguration config;
        config.requestTimeoutMs = 15 * 1000;
        config.region = Aws::Region::US_EAST_1;
        return config;
    }

    static Aws::Client::ClientConfiguration create_lambda_config()
    {
        Aws::Client::ClientConfiguration config;
        config.requestTimeoutMs = 15 * 1000;
        config.region = Aws::Environment::GetEnv("AWS_REGION");
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
        Model::CreateFunctionRequest createFunctionRequest;
        createFunctionRequest.SetHandler(handler_name);
        createFunctionRequest.SetFunctionName(function_name);
        // I ran into eventual-consistency issues when creating the role dynamically as part of the test.
        createFunctionRequest.SetRole(get_role_arn("integration-tests"));
        Model::FunctionCode funcode;
        funcode.WithS3Bucket(S3BUCKET).WithS3Key(build_resource_name(S3KEY));
        createFunctionRequest.SetCode(funcode);
        createFunctionRequest.SetRuntime(Aws::Lambda::Model::Runtime::provided);

        auto outcome = m_lambda_client.CreateFunction(createFunctionRequest);
        ASSERT_TRUE(outcome.IsSuccess()) << "Failed to create function " << function_name;
    }

    void delete_function(Aws::String const& function_name, bool assert = true)
    {
        Model::DeleteFunctionRequest deleteFunctionRequest;
        deleteFunctionRequest.SetFunctionName(function_name);
        auto outcome = m_lambda_client.DeleteFunction(deleteFunctionRequest);
        if (assert) {
            ASSERT_TRUE(outcome.IsSuccess()) << "Failed to delete function " << function_name;
        }
    }
};

TEST_F(LambdaRuntimeTest, echo_success)
{
    Aws::String const funcname = build_resource_name("echo_success");
    char const payloadContent[] = "Hello, Lambda!";
    create_function(funcname, "echo_success" /*handler_name*/);
    Model::InvokeRequest invokeRequest;
    invokeRequest.SetFunctionName(funcname);
    invokeRequest.SetInvocationType(Model::InvocationType::RequestResponse);
    invokeRequest.SetContentType("application/json");

    std::shared_ptr<Aws::IOStream> payload = Aws::MakeShared<Aws::StringStream>("FunctionTest");
    Aws::Utils::Json::JsonValue jsonPayload;
    jsonPayload.WithString("barbaz", payloadContent);
    *payload << jsonPayload.View().WriteCompact();
    invokeRequest.SetBody(payload);

    Model::InvokeOutcome invokeOutcome = m_lambda_client.Invoke(invokeRequest);
    EXPECT_TRUE(invokeOutcome.IsSuccess());
    Aws::StringStream output;
    if (!invokeOutcome.IsSuccess()) {
        delete_function(funcname);
        return;
    }
    EXPECT_EQ(200, invokeOutcome.GetResult().GetStatusCode());
    EXPECT_TRUE(invokeOutcome.GetResult().GetFunctionError().empty());
    auto const jsonResponse = Aws::Utils::Json::JsonValue(invokeOutcome.GetResult().GetPayload());
    EXPECT_TRUE(jsonResponse.WasParseSuccessful());
    EXPECT_STREQ(payloadContent, jsonResponse.View().GetString("barbaz").c_str());
    delete_function(funcname);
}

TEST_F(LambdaRuntimeTest, echo_unicode)
{
    Aws::String const funcname = build_resource_name("echo_success"); // re-use the echo method but with Unicode input
    char const payloadContent[] = "画像は1000語の価値がある";
    create_function(funcname, "echo_success" /*handler_name*/);
    Model::InvokeRequest invokeRequest;
    invokeRequest.SetFunctionName(funcname);
    invokeRequest.SetInvocationType(Model::InvocationType::RequestResponse);
    invokeRequest.SetContentType("application/json");

    std::shared_ptr<Aws::IOStream> payload = Aws::MakeShared<Aws::StringStream>("FunctionTest");
    Aws::Utils::Json::JsonValue jsonPayload;
    jsonPayload.WithString("UnicodeText", payloadContent);
    *payload << jsonPayload.View().WriteCompact();
    invokeRequest.SetBody(payload);

    Model::InvokeOutcome invokeOutcome = m_lambda_client.Invoke(invokeRequest);
    EXPECT_TRUE(invokeOutcome.IsSuccess());
    Aws::StringStream output;
    if (!invokeOutcome.IsSuccess()) {
        delete_function(funcname);
        return;
    }
    EXPECT_EQ(200, invokeOutcome.GetResult().GetStatusCode());
    EXPECT_TRUE(invokeOutcome.GetResult().GetFunctionError().empty());
    auto const jsonResponse = Aws::Utils::Json::JsonValue(invokeOutcome.GetResult().GetPayload());
    EXPECT_TRUE(jsonResponse.WasParseSuccessful());
    EXPECT_STREQ(payloadContent, jsonResponse.View().GetString("UnicodeText").c_str());
    delete_function(funcname);
}

TEST_F(LambdaRuntimeTest, echo_failure)
{
    Aws::String const funcname = build_resource_name("echo_failure");
    create_function(funcname, "echo_failure" /*handler_name*/);
    Model::InvokeRequest invokeRequest;
    invokeRequest.SetFunctionName(funcname);
    invokeRequest.SetInvocationType(Model::InvocationType::RequestResponse);

    Model::InvokeOutcome invokeOutcome = m_lambda_client.Invoke(invokeRequest);
    EXPECT_TRUE(invokeOutcome.IsSuccess());
    EXPECT_EQ(200, invokeOutcome.GetResult().GetStatusCode());
    EXPECT_STREQ("Unhandled", invokeOutcome.GetResult().GetFunctionError().c_str());
    delete_function(funcname);
}

TEST_F(LambdaRuntimeTest, binary_response)
{
    Aws::String const funcname = build_resource_name("binary_response");
    unsigned long constexpr expected_length = 1451;
    create_function(funcname, "binary_response" /*handler_name*/);
    Model::InvokeRequest invokeRequest;
    invokeRequest.SetFunctionName(funcname);
    invokeRequest.SetInvocationType(Model::InvocationType::RequestResponse);

    Model::InvokeOutcome invokeOutcome = m_lambda_client.Invoke(invokeRequest);
    EXPECT_TRUE(invokeOutcome.IsSuccess());
    EXPECT_EQ(200, invokeOutcome.GetResult().GetStatusCode());
    EXPECT_TRUE(invokeOutcome.GetResult().GetFunctionError().empty());
    EXPECT_EQ(expected_length, invokeOutcome.GetResult().GetPayload().tellp());
    delete_function(funcname);
}
} // namespace
