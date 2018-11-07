#include <gtest/gtest.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/utils/Outcome.h>
#include <aws/core/utils/memory/stl/AWSString.h>
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
        // config.region = Aws::Region::US_WEST_2;
        config.region = Aws::Region::EU_WEST_1;
        return config;
    }

    LambdaRuntimeTest() : m_client(create_lambda_config()) {}

    ~LambdaRuntimeTest()
    {
        // clean up in case we exited one test abnormally
        delete_function("echo_success", false /*assert*/);
        delete_function("echo_failure", false /*assert*/);
    }

    void create_function(Aws::String const& name)
    {
        Model::CreateFunctionRequest createFunctionRequest;
        createFunctionRequest.SetHandler(name);
        createFunctionRequest.SetFunctionName(name);
        createFunctionRequest.SetRole("arn:aws:iam::383981755760:role/lambda-lab"); // TODO: create this dynamically
        Model::FunctionCode funcode;
        Aws::String s3key(name);
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
    create_function(funcname);
    Model::InvokeRequest invokeRequest;
    invokeRequest.SetFunctionName(funcname);
    invokeRequest.SetInvocationType(Model::InvocationType::Event);

    Model::InvokeOutcome invokeOutcome = m_client.Invoke(invokeRequest);
    EXPECT_TRUE(invokeOutcome.IsSuccess());
    EXPECT_EQ(202, invokeOutcome.GetResult().GetStatusCode());
    EXPECT_TRUE(invokeOutcome.GetResult().GetFunctionError().empty());
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

TEST(sample, happy_case)
{
    ASSERT_TRUE(true);
}
