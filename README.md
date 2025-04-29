[![GitHub](https://img.shields.io/github/license/awslabs/aws-lambda-cpp.svg)](https://github.com/awslabs/aws-lambda-cpp/blob/master/LICENSE)
![Code Quality badge](https://github.com/awslabs/aws-lambda-cpp/actions/workflows/code-quality.yml/badge.svg)

| OS | Arch | Status |
|----|------|--------|
| Amazon Linux 2 | x86_64 | [![](https://codebuild.us-west-2.amazonaws.com/badges?uuid=eyJlbmNyeXB0ZWREYXRhIjoiQ1EvQXE0ODBLK0VnQitMaVdvZ1J0QkhTMlpNbk8wS0lRbWZvRDlPSHB0V0VXb1VLazdSdzRMWHhMeUdpYjdOT1hCc1hjL3BKei96ZVpzeTdrMVd4c3BRPSIsIml2UGFyYW1ldGVyU3BlYyI6IkhjTTNoSzJwb1hldk9zZFYiLCJtYXRlcmlhbFNldFNlcmlhbCI6MX0%3D&branch=master)](https://us-west-2.codebuild.aws.amazon.com/project/eyJlbmNyeXB0ZWREYXRhIjoicnpvbytDV0grMHh2c09ONi9kQ3ZuOVVwckxISElKRENEVy9CL0pvd3VvLzQwZ21pdzBOdGtNWUFLRy9VRkw1NldSMmRlVXV5R0NhN1k1OWI0bDY1N2MyMzR2SmhseWlma0hmWTlBUkwzcVp0TEJlQm1RPT0iLCJpdlBhcmFtZXRlclNwZWMiOiI3MzM4WDUybk9hSkl1bllRIiwibWF0ZXJpYWxTZXRTZXJpYWwiOjF9) |
| Amazon Linux 2 | aarch64 | [![](https://codebuild.us-west-2.amazonaws.com/badges?uuid=eyJlbmNyeXB0ZWREYXRhIjoicWNGSmJtaGdPSCtqR25KQ1k3RWNZS1pwWlZScGZ3WU1JM0lISnZJVkhVNy8zbVIyVHp6RlBmRjN4cjZJd2xWNEd0eWZmUy9JaE1vRzBYWFcrbnpFdDUwPSIsIml2UGFyYW1ldGVyU3BlYyI6ImVoeHl5TTNtMmdERjJuWisiLCJtYXRlcmlhbFNldFNlcmlhbCI6MX0%3D&branch=master)](https://us-west-2.codebuild.aws.amazon.com/project/eyJlbmNyeXB0ZWREYXRhIjoiVUVaNzBYMXVjUUl1djdlS3pTSXVxMUhKcHB4ZC96ZjlDOWM3bUxiRmtITnVGYzdxTDJveFY3eVFqanpHbzhYRUdWVjVhZFhnOGt0NldETEVMamN0alRoZzYwMyszU1lVMjJNR0lUWGNCQjVYNzhuUzZwZ0ptZz09IiwiaXZQYXJhbWV0ZXJTcGVjIjoicmtKaUVoM2pmUVdibVZuOSIsIm1hdGVyaWFsU2V0U2VyaWFsIjoxfQ%3D%3D) |
| Amazon Linux (ALAMI) | x86_64 | [![](https://codebuild.us-west-2.amazonaws.com/badges?uuid=eyJlbmNyeXB0ZWREYXRhIjoiWUNqeG9FcmUyQzVSaUkydFd6UkU5Sm42cTViSExXOFZURHRBQlM0dDJDOThMWEFYLzN4NitQR0w1ZzNKcjAwOVNUYXY5ZUljU1hzcEtrU0N0dEhUN0M0PSIsIml2UGFyYW1ldGVyU3BlYyI6ImtYU0ZjSzh3ekFKazlBVVUiLCJtYXRlcmlhbFNldFNlcmlhbCI6MX0%3D&branch=master)](https://us-west-2.codebuild.aws.amazon.com/project/eyJlbmNyeXB0ZWREYXRhIjoiTEJJVVFIOXp6VjUvWExqODN1K1NPQmRTVm9iQy9ZK2tmKzkrbVdTNlh1LzV1UlpQL2lPN1Faak0yc0pOaGpEVlRpai9yS3JCRjBRQU5lMVFVU1hRU1hyekxpVi8yNWV0ZE44SElWdlRpNld4bmkwdE1oQjcxN0NtIiwiaXZQYXJhbWV0ZXJTcGVjIjoiZnBBUi9uOU8yVjJ4RENpRyIsIm1hdGVyaWFsU2V0U2VyaWFsIjoxfQ%3D%3D) |
| Alpine | x86_64 | [![](https://codebuild.us-west-2.amazonaws.com/badges?uuid=eyJlbmNyeXB0ZWREYXRhIjoiTkhhOEJGNjVOTG5NZWVNWDNjSGNEdWEwY0J2ZUNLMkE2aU83UVdYc3VMU0V5b1JqdXY0OXUxNkxYRDUxU0VJOTByL3NLUTE3djBMNWh2VldXdk0xamJZPSIsIml2UGFyYW1ldGVyU3BlYyI6ImQxSjc2Vnd3czF2QWphRS8iLCJtYXRlcmlhbFNldFNlcmlhbCI6MX0%3D&branch=master)](https://us-west-2.codebuild.aws.amazon.com/project/eyJlbmNyeXB0ZWREYXRhIjoiQzJVUzZML1dLTkpRNGcxSjVyUXVEd1BCY2poZUhydWZLeGE5MGU1c05vNDVObG44bnpKZFhlZVJKSm50ZnpaalRENUxxOHpPNGdPTDRlTGc4WW81UHd4L3hCeTgyTm5vRVR0RW5FempKdk00aDlPRk02WGQiLCJpdlBhcmFtZXRlclNwZWMiOiJUMFhCQktLMExQMXc3Q0lHIiwibWF0ZXJpYWxTZXRTZXJpYWwiOjF9) |
| Arch Linux | x86_64 | [![](https://codebuild.us-west-2.amazonaws.com/badges?uuid=eyJlbmNyeXB0ZWREYXRhIjoib2cxaHp3bE5ndWhWR0RIRkxxQzRwR1dHa05DWmQ0bENnWGNHYzM2YmR3OFRHNWpPYStGYUM1WXBQVUNoZjJRa2xrZVpuRXVyWVVvQVNzNExqSlN5TGEwPSIsIml2UGFyYW1ldGVyU3BlYyI6Ii9zSjVybGNsNEJMUEZwSlUiLCJtYXRlcmlhbFNldFNlcmlhbCI6MX0%3D&branch=master)](https://us-west-2.codebuild.aws.amazon.com/project/eyJlbmNyeXB0ZWREYXRhIjoiRWVOYlA5OHZqUVVLUTZLYlJzZmdOQkR5dmpVSTBPS1h1M3RxQkxXa3pyMC9OOUw5dDJlUDcyYm05Q3pBOEZ1aWJFYkFBajFGZ3RJWUM5WkpoZUE4K0IrdFIvYytyNVRYREpQTUNHL05vTXlLQ0E9PSIsIml2UGFyYW1ldGVyU3BlYyI6InFuS1hJY3JTaWpSWENLM1EiLCJtYXRlcmlhbFNldFNlcmlhbCI6MX0%3D) |
| Ubuntu 18.04 | x86_64 | [![](https://codebuild.us-west-2.amazonaws.com/badges?uuid=eyJlbmNyeXB0ZWREYXRhIjoiVkhsbmdlYkk3M1JESVdiTHc0elpobXEvUk4wRWlBZUpEZzdmem1QbGJRZ3dMbVE2RWZpbHZjNmVCd0dJaUFSZ1pzQVlyZ1dvdndWTjZSRjg0WDRYRFh3PSIsIml2UGFyYW1ldGVyU3BlYyI6IjJic2dnR3ZpTEQyMmRPMXQiLCJtYXRlcmlhbFNldFNlcmlhbCI6MX0%3D&branch=master)](https://us-west-2.codebuild.aws.amazon.com/project/eyJlbmNyeXB0ZWREYXRhIjoiSlNPak1vQmVBR3JnUlAwRWg2N3hHRHF1U2Z6RkQvY1NHRHM4RTJ0WEFBdjFTSzBzY21kZEpPMDk2QXdwRStUWUZmWWFmTkRkU1FGa0lQUGoxbU9GNU45QVJ1YVkzZkY0dmsxV2FRZVljakt3UmJpdTM2a0JnQT09IiwiaXZQYXJhbWV0ZXJTcGVjIjoieE5LSUlmNVN1UWdqbWg0cSIsIm1hdGVyaWFsU2V0U2VyaWFsIjoxfQ%3D%3D) |

## AWS Lambda C++ Runtime

C++ implementation of the lambda runtime API

## Design Goals
1. Negligible cold-start overhead (single digit millisecond).
2. Freedom of choice in compilers, build platforms and C standard library versions.

## Building and Installing the Runtime
Since AWS Lambda runs on GNU/Linux, you should build this runtime library and your logic on GNU/Linux as well.

### Prerequisites
Make sure you have the following packages installed first:
1. CMake (version 3.9 or later)
1. git
1. Make or Ninja
1. zip
1. libcurl-devel (on Debian-based distros it's libcurl4-openssl-dev)

In a terminal, run the following commands:
```bash
$ git clone https://github.com/awslabs/aws-lambda-cpp.git
$ cd aws-lambda-cpp
$ mkdir build
$ cd build
$ cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=~/lambda-install
$ make && make install
```

### Running Unit Tests Locally

To run the unit tests locally, follow these steps to build:

```bash
$ cd aws-lambda-cpp
$ mkdir build
$ cd build
$ cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_TESTS=ON
$ make
```

Run unit tests:
```bash
$ ctest
```

To consume this library in a project that is also using CMake, you would do:

```cmake
cmake_minimum_required(VERSION 3.9)
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

And here is how a sample `main.cpp` would look like:
```cpp
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
```

And finally, here's how you would package it all. Run the following commands from your application's root directory:

```bash
$ mkdir build
$ cd build
$ cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=~/lambda-install
$ make
$ make aws-lambda-package-demo
```
The last command above `make aws-lambda-package-demo` will create a zip file called `demo.zip` in the current directory.

Now, create an IAM role and the Lambda function via the AWS CLI.

First create the following trust policy JSON file

```
$ cat trust-policy.json
{
 "Version": "2012-10-17",
  "Statement": [
    {
      "Effect": "Allow",
      "Principal": {
        "Service": ["lambda.amazonaws.com"]
      },
      "Action": "sts:AssumeRole"
    }
  ]
}

```
Then create the IAM role:

```bash
$ aws iam create-role --role-name lambda-demo --assume-role-policy-document file://trust-policy.json
```

Note down the role Arn returned to you after running that command. We'll need it in the next steps:

Attach the following policy to allow Lambda to write logs in CloudWatch:
```bash
$ aws iam attach-role-policy --role-name lambda-demo --policy-arn arn:aws:iam::aws:policy/service-role/AWSLambdaBasicExecutionRole
```

Make sure you attach the appropriate policies and/or permissions for any other AWS services that you plan on using.

And finally, create the Lambda function:

```
$ aws lambda create-function --function-name demo \
--role <specify role arn from previous step here> \
--runtime provided.al2023 --timeout 15 --memory-size 128 \
--handler demo --zip-file fileb://demo.zip
```
> **N.B.** If you are building on `arm64`, you have to explicitly add the param `--architectures arm64`, so that you are setting up the proper architecture on AWS to run your supplied Lambda function.

And to invoke the function:
```bash
$ aws lambda invoke --function-name demo --cli-binary-format raw-in-base64-out --payload '{"answer":42}' output.txt
```

You can update your supplied function:
```bash
$ aws lambda update-function-code --function-name demo --zip-file fileb://demo.zip
```

## Using the C++ SDK for AWS with this runtime
This library is completely independent from the AWS C++ SDK. You should treat the AWS C++ SDK as just another dependency in your application.
See [the examples section](https://github.com/awslabs/aws-lambda-cpp/tree/master/examples/) for a demo utilizing the AWS C++ SDK with this Lambda runtime.

## Supported Compilers
Any *fully* compliant C++11 compiler targeting GNU/Linux x86-64 should work. Please avoid compiler versions that provide half-baked C++11 support.

- Use GCC v5.x or above
- Use Clang v3.3 or above

## Packaging, ABI, GNU C Library, Oh My!
Lambda runs your code on some version of Amazon Linux. It would be a less than ideal customer experience if you are forced to build your application on that platform and that platform only.

However, the freedom to build on any linux distro brings a challenge. The GNU C Library ABI. There is no guarantee the platform used to build the Lambda function has the same GLIBC version as the one used by AWS Lambda. In fact, you might not even be using GNU's implementation. For example you could build a C++ Lambda function using musl libc.

To ensure that your application will run correctly on Lambda, we must package the entire C runtime library with your function.
If you choose to build on the same [Amazon Linux version used by lambda](https://docs.aws.amazon.com/lambda/latest/dg/current-supported-versions.html), you can avoid packaging the C runtime in your zip file.
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
If you are not using the AWS C++ SDK, but happen to be using libcurl directly, you can set the CA bundle location by doing:
```c
curl_easy_setopt(curl_handle, CURLOPT_CAINFO, "/etc/pki/tls/certs/ca-bundle.crt");
```

## FAQ & Troubleshooting
1. **Why is the zip file so large? what are all those files?**
   Typically, the zip file is large because we have to package the entire C standard library.
   You can reduce the size by doing some or all of the following:
   - Ensure you're building in release mode `-DCMAKE_BUILD_TYPE=Release`
   - If possible, build your function using musl libc, it's tiny. The easiest way to do this, assuming your code is portable, is to build on Alpine linux, which uses musl libc by default.
1. **How to upload a zip file that's bigger than 50MB via the CLI?**
    Upload your zip file to S3 first:
    ```bash
    $ aws s3 cp demo.zip s3://mys3bucket/demo.zip
    ```
    NOTE: you must use the same region for your S3 bucket as the lambda.

    Then you can create the Lambda function this way:

    ```bash
    $ aws lambda create-function --function-name demo \
    --role <specify role arn here> \
    --runtime provided.al2023 --timeout 15 --memory-size 128 \
    --handler demo
    --code "S3Bucket=mys3bucket,S3Key=demo.zip"
    ```
> **N.B.** See hint above if you are building on `arm64`.   

1. **My code is crashing, how can I debug it?**

   - Starting with [v0.2.0](https://github.com/awslabs/aws-lambda-cpp/releases/tag/v0.2.0) you should see a stack-trace of the crash site in the logs (which are typically stored in CloudWatch).
     - To get a more detailed stack-trace with source-code information such as line numbers, file names, etc. you need to install one of the following packages:
       - On Debian-based systems -  `sudo apt install libdw-dev` or `sudo apt install binutils-dev`
       - On RHEL based systems -  `sudo yum install elfutils-devel` or `sudo yum install binutils-devel`
       If you have either of those packages installed, CMake will detect them and automatically link to them. No other
       steps are required.
   - Turn up the logging verbosity to the maximum.
     - Build the runtime in Debug mode. `-DCMAKE_BUILD_TYPE=Debug`. Verbose logs are enabled by default in Debug builds.
     - To enable verbose logs in Release builds, build the runtime with the following CMake flag `-DLOG_VERBOSITY=3`
     - If you are using the AWS C++ SDK, see [this FAQ](https://github.com/aws/aws-sdk-cpp/wiki#how-do-i-turn-on-logging) on how to adjust its logging verbosity
   - Run your code locally on an Amazon Linux AMI or Docker container to reproduce the problem
     - If you go the AMI route, [use the official one](https://docs.aws.amazon.com/lambda/latest/dg/current-supported-versions.html) recommended by AWS Lambda 
     - If you go the Docker route, use the following command to launch a container running AL2017.03
       `$ docker run -v /tmp:/tmp -it --security-opt seccomp=unconfined amazonlinux:2017.03`
       The `security-opt` argument is necessary to run `gdb`, `strace`, etc.
1. **CURL problem with the SSL CA cert**
   - Make sure you are using a `libcurl` version built with OpenSSL, or one of its flavors (BoringSSL, LibreSSL)
   - Make sure you tell `libcurl` where to find the CA bundle file.
   - You can try hitting the non-TLS version of the endpoint if available. (Not Recommended).
1. **No known conversion between `std::string` and `Aws::String`**
    - Either turn off custom memory management in the AWS C++ SDK or build it as a static library (`-DBUILD_SHARED_LIBS=OFF`)

## License

This library is licensed under the Apache 2.0 License. 
