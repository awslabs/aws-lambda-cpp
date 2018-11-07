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

static const char S3BUCKET[] = "aws-lambda-cpp-runtime-tests";

struct LambdaRuntimeTest : public ::testing::Test {
    LambdaClient m_client;
    static Aws::Client::ClientConfiguration create_lambda_config()
    {
        Aws::Client::ClientConfiguration config;
        config.requestTimeoutMs = 15 * 1000;
        config.region = Aws::Region::US_WEST_2;
        return config;
    }

    LambdaRuntimeTest() : m_client(create_lambda_config()) { create_function("echo_success"); }

    ~LambdaRuntimeTest() { delete_function("echo_success"); }

    void create_function(char const* name)
    {
        Model::CreateFunctionRequest createFunctionRequest;
        createFunctionRequest.SetHandler(name);
        createFunctionRequest.SetFunctionName(name);
        createFunctionRequest.SetRole("arn:aws:iam::383981755760:role/lambda-lab"); // TODO: create this dynamically
        Model::FunctionCode funcode;
        Aws::String s3key(name);
        funcode.WithS3Bucket(S3BUCKET).WithS3Key(s3key + ".zip");
        createFunctionRequest.SetCode(funcode);
        createFunctionRequest.SetRuntime(Aws::Lambda::Model::Runtime::provided);

        auto outcome = m_client.CreateFunction(createFunctionRequest);
        ASSERT_TRUE(outcome.IsSuccess());
    }

    void delete_function(char const* name)
    {
        Model::DeleteFunctionRequest deleteFunctionRequest;
        deleteFunctionRequest.SetFunctionName(name);
        auto outcome = m_client.DeleteFunction(deleteFunctionRequest);
        ASSERT_TRUE(outcome.IsSuccess());
    }
};

TEST_F(LambdaRuntimeTest, echo_success)
{
    Aws::String funcname = "echo_success";
    Model::InvokeRequest invokeRequest;
    invokeRequest.SetFunctionName(funcname);
    invokeRequest.SetInvocationType(Model::InvocationType::Event);

    Model::InvokeOutcome invokeOutcome = m_client.Invoke(invokeRequest);
    ASSERT_TRUE(invokeOutcome.IsSuccess());
    ASSERT_EQ(202,invokeOutcome.GetResult().GetStatusCode());

}

TEST(sample, happy_case)
{
    ASSERT_TRUE(true);
}
