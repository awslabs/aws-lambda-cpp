## AWS Lambda C++ Runtime

C++ implementation of the lambda runtime API

## Building The Runtime
Since AWS Lambda runs on GNU/Linux, you should build this runtime library and your logic on GNU/Linux as well.


Using CMake, run the following commands:
```cmake
$ git clone https://github.com/awslabs/aws-lambda-cpp-runtime.git
$ cd aws-lambda-cpp-runtime
$ mkdir build
$ cd build
$ cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=~/lambda-install
$ make && make install
```

To consume this library in a project that is also using CMake, you would do:

```cmake
cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 11)
project(demo LANGUAGES CXX)
find_package(aws-lambda-runtime)
add_executable(${PROJECT_NAME} "main.cpp")
target_link_libraries(${PROJECT_NAME} PRIVATE AWS::aws-lambda-runtime)
target_compile_features(${PROJECT_NAME} PRIVATE "cxx_std_11")
target_compile_options(${PROJECT_NAME} PRIVATE "-Wall" "-Wextra")

# this line creates a target that packages your binary and zips it up
aws_lambda_package_target(${PROJECT_NAME})
```

And here is how a sample `main.cpp` would look like for example:
```cpp
#include <aws/lambda-runtime/runtime.h>

using namespace aws::lambda_runtime;

static invocation_response my_handler(invocation_request const& req)
{
    if (req.payload.length() > 42) {
        return invocation_response::failure("error message here"/*error_message*/, "error type here" /*error_type*/);
    }

    return invocation_response::success("json payload here" /*payload*/, "application/json" /*MIME type*/);
}

int main()
{
    run_handler(my_handler);
    return 0;
}
```

And finally, here's how you would package it all and send it to AWS Lambda. Run the following commands
from your application's root directory:

```bash
$ mkdir build
$ cd build
$ cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=~/lambda-install
$ make
$ make aws-lambda-package-demo
```
The last command above `make aws-lambda-package-demo` will create a zip file called `demo.zip` in the current directory.

Using the aws CLI, create a lambda function that uses that zip file as such:

```bash
$ aws lambda create-function --function-name demo \
--role <specify role arn here> \
--runtime provided --timeout 15 --memory-size 128 \
--handler demo --zip-file fileb://demo.zip
```

And to invoke the function:
```bash
$ aws lambda invoke --function-name demo --payload '{"answer":42}' output.txt
```

## Using the C++ SDK for AWS with this runtime
This library is completely independent from the AWS C++ SDK. You should treat the AWS C++ SDK as just another dependency in your application.
This is a [detailed example](https://github.com/awslabs/aws-lambda-cpp-runtime/examples/README.md) of using the AWS C++ SDK with this Lambda runtime.

## Supported Compilers
We've tested with Clang & GCC but in theory any *fully* compliant C++11 compiler targeting GNU/Linux x86-64 should work.

## Packaging, ABI, GNU C Library, Oh My!
Lambda runs your code on some version of Amazon Linux. It would be a less than ideal customer  experience if you are forced to build your application on that platform and that platform only.

However, the freedom to build on any linux distro brings a challenge. The GNU C Library ABI. There is no guarantee the platform used to build the Lambda function has the same GLIBC version as the one used by AWS Lambda. In fact, you might not even be using GNU's implementation. For example you could build a C++ Lambda function using musl libc.

To ensure that your application will run correctly on Lambda, , we must package the entire C runtime library with your function.
If you choose to build on the same Amazon Linux version used by lambda, you could avoid packaging the C runtime in your zip file.
This can be done by passing the `NO_LIBC` flag in CMake as follows:
```cmake
aws_lambda_package_target(${PROJECT_NAME} NO_LIBC)
```
### Common Pitfalls with Packaging

* Any library dependency your Lambda function has that is dynamically loaded via `dlopen` will NOT be automatically packaged. You **must** add those dependencies manually to the zip file.
This applies to any configuration or resource files that your code depends on.

* If you are making HTTP calls over TLS (https), keep in mind that the CA bundle location is different between distros.
For example, if you are using the AWS C++ SDK, it's best to set the following configuration options:

```cpp
Aws::Client::ClientConfiguration config;
config.caFile = "/etc/pki/tls/certs/ca-bundle.crt";
```
If you are not using the AWS C++ SDK, but happens to be using libcurl directly, you can set the CA bundle location by doing:
```c
curl_easy_setopt(curl_handle, CURLOPT_CAINFO, "/etc/pki/tls/certs/ca-bundle.crt");
```

## FAQ
* Why is the zip file so large? what are all those files?
Typically, the zip file is large because we have to package the entire C standard library.
You can reduce the size by doing some or all of the following:
1. Ensure you're building in release mode `-DCMAKE_BUILD_TYPE=Release`
1. If possible, build your function using musl libc, it's tiny. The easiest way to do this, assuming your code is portable, is to build on Alpine linux.
* How to upload a zip file that's bigger than 50MB via the CLI?
Upload your zip file to S3 first:
```bash
$ aws s3 cp demo.zip s3://mys3bucket/demo.zip
```
NOTE: you must use the same region for your S3 bucket as the lambda.

Then tell Lambda where to get it from:
```bash
$ aws lambda create-function --function-name demo \
--role <specify role arn here> \
--runtime provided --timeout 15 --memory-size 128 \
--handler demo
--code "S3Bucket=mys3bucket,S3Key=demo.zip"
```

## License
This library is licensed under the Apache 2.0 License. 
