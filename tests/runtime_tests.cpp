#include <gtest/gtest.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/utils/Outcome.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/core/utils/memory/stl/AWSStringStream.h>
#include <aws/lambda/LambdaClient.h>
#include <aws/lambda/model/DeleteFunctionRequest.h>
#include <aws/lambda/model/GetFunctionRequest.h>
#include <aws/lambda/model/CreateFunctionRequest.h>
#include <aws/lambda/model/DeleteFunctionRequest.h>
#include <aws/lambda/model/InvokeRequest.h>

using namespace Aws::Lambda;

static const char S3BUCKET[] = "aws-lambda-cpp-runtime-tests-eu";
static const char S3KEY[] = "lambda-tests.zip";

struct LambdaRuntimeTest : public ::testing::Test {
    LambdaClient m_client;
    static Aws::Client::ClientConfiguration create_lambda_config()
    {
        Aws::Client::ClientConfiguration config;
        config.requestTimeoutMs = 15 * 1000;
        config.region = Aws::Region::EU_WEST_1;
        return config;
    }

    LambdaRuntimeTest() : m_client(create_lambda_config()) {}

    ~LambdaRuntimeTest() override
    {
        // clean up in case we exited one test abnormally
        delete_function("echo_success", false /*assert*/);
        delete_function("echo_failure", false /*assert*/);
        delete_function("binary_response", false /*assert*/);
    }

    void create_function(Aws::String const& name)
    {
        Model::CreateFunctionRequest createFunctionRequest;
        createFunctionRequest.SetHandler(name);
        createFunctionRequest.SetFunctionName(name);
        createFunctionRequest.SetRole("arn:aws:iam::383981755760:role/lambda-lab"); // TODO: create this dynamically
        Model::FunctionCode funcode;
        funcode.WithS3Bucket(S3BUCKET).WithS3Key(S3KEY);
        createFunctionRequest.SetCode(funcode);
        createFunctionRequest.SetRuntime(Aws::Lambda::Model::Runtime::provided);

        auto outcome = m_client.CreateFunction(createFunctionRequest);
        ASSERT_TRUE(outcome.IsSuccess()) << "Failed to create function " << name;
    }

    void delete_function(Aws::String const& name, bool assert = true)
    {
        Model::DeleteFunctionRequest deleteFunctionRequest;
        deleteFunctionRequest.SetFunctionName(name);
        auto outcome = m_client.DeleteFunction(deleteFunctionRequest);
        if (assert) {
            ASSERT_TRUE(outcome.IsSuccess()) << "Failed to delete function " << name;
        }
    }
};

TEST_F(LambdaRuntimeTest, echo_success)
{
    Aws::String const funcname = "echo_success";
    char const payloadContent[] = "Hello, Lambda!";
    create_function(funcname);
    Model::InvokeRequest invokeRequest;
    invokeRequest.SetFunctionName(funcname);
    invokeRequest.SetInvocationType(Model::InvocationType::RequestResponse);
    invokeRequest.SetContentType("application/json");

    std::shared_ptr<Aws::IOStream> payload = Aws::MakeShared<Aws::StringStream>("FunctionTest");
    Aws::Utils::Json::JsonValue jsonPayload;
    jsonPayload.WithString("barbaz", payloadContent);
    *payload << jsonPayload.View().WriteCompact();
    invokeRequest.SetBody(payload);

    Model::InvokeOutcome invokeOutcome = m_client.Invoke(invokeRequest);
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
    Aws::String const funcname = "echo_success"; // re-use the echo method but with Unicode input
    char const payloadContent[] = "画像は1000語の価値がある";
    create_function(funcname);
    Model::InvokeRequest invokeRequest;
    invokeRequest.SetFunctionName(funcname);
    invokeRequest.SetInvocationType(Model::InvocationType::RequestResponse);
    invokeRequest.SetContentType("application/json");

    std::shared_ptr<Aws::IOStream> payload = Aws::MakeShared<Aws::StringStream>("FunctionTest");
    Aws::Utils::Json::JsonValue jsonPayload;
    jsonPayload.WithString("UnicodeText", payloadContent);
    *payload << jsonPayload.View().WriteCompact();
    invokeRequest.SetBody(payload);

    Model::InvokeOutcome invokeOutcome = m_client.Invoke(invokeRequest);
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
    Aws::String const funcname = "echo_failure";
    create_function(funcname);
    Model::InvokeRequest invokeRequest;
    invokeRequest.SetFunctionName(funcname);
    invokeRequest.SetInvocationType(Model::InvocationType::RequestResponse);

    Model::InvokeOutcome invokeOutcome = m_client.Invoke(invokeRequest);
    EXPECT_TRUE(invokeOutcome.IsSuccess());
    EXPECT_EQ(200, invokeOutcome.GetResult().GetStatusCode());
    EXPECT_STREQ("Unhandled", invokeOutcome.GetResult().GetFunctionError().c_str());
    delete_function(funcname);
}

TEST_F(LambdaRuntimeTest, binary_response)
{
    Aws::String const funcname = "binary_response";
    unsigned long constexpr expected_length = 1451;
    create_function(funcname);
    Model::InvokeRequest invokeRequest;
    invokeRequest.SetFunctionName(funcname);
    invokeRequest.SetInvocationType(Model::InvocationType::RequestResponse);

    Model::InvokeOutcome invokeOutcome = m_client.Invoke(invokeRequest);
    EXPECT_TRUE(invokeOutcome.IsSuccess());
    EXPECT_EQ(200, invokeOutcome.GetResult().GetStatusCode());
    EXPECT_TRUE(invokeOutcome.GetResult().GetFunctionError().empty());
    EXPECT_EQ(expected_length, invokeOutcome.GetResult().GetPayload().tellp());
    delete_function(funcname);
}
