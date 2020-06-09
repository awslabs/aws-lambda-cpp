# Example using the AWS C++ Lambda runtime and Amazon API Gateway

In this example, we'll build a simple "Hello, World" lambda function that can be invoked using an api endpoint created using Amazon API gateway. This example can be viewed as the C++ counterpart to the NodeJS "Hello, World" API example as viewed [here](https://docs.aws.amazon.com/apigateway/latest/developerguide/api-gateway-create-api-as-simple-proxy-for-lambda.html). At the end of this example, you should be able to invoke your lambda via an api endpoint and receive a raw JSON response. For brevity, we will only use the C++ Lambda runtime in this example and omit the use of the AWS C++ SDK. 

## Build the Runtime
We first need to build the C++ Lambda runtime as outlined in the other examples.

```bash
$ git clone https://github.com/awslabs/aws-lambda-cpp-runtime.git
$ cd aws-lambda-cpp-runtime
$ mkdir build
$ cd build
$ cmake .. -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=OFF \
  -DCMAKE_INSTALL_PREFIX=~/install \
$ make
$ make install
```

## Build the application
The next step is to build the Lambda function in `main.cpp` and run the packaging command as follows:

```bash
$ mkdir build
$ cd build
$ cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=~/install
$ make
$ make aws-lambda-package-api
```

You should now have a zip file called `api.zip`. Follow the instructions in the main README to upload it and return here once complete.

## Using Amazon API Gateway 
For the rest of this example, we will use the AWS Management Console to create the API endpoint using Amazon API Gateway.

(i) Navigate to AWS Lambda within the console [here](https://console.aws.amazon.com/lambda/home)   
(ii) Select the newly created function. Within the specific function, the "Designer" window should appear.   
(iii) Simply click "Add trigger" -> "API Gateway" -> "Create an API". [Here](https://www.amazon.com/photos/shared/izUpPmCbRHKnpeaqVI-9Fw.CnypCJwUl-vsp5YbSGxF9R) are some sample settings you can populate the configuration with.   
(iv) Once you have added the API gateway, locate the newly created endpoint. View the `main.cpp` file on what request is expected.   
