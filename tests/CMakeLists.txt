project(aws-lambda-runtime-tests LANGUAGES CXX)
find_package(aws-cpp-sdk-lambda REQUIRED)
find_package(GTest REQUIRED)
find_package(CURL)

add_executable(${PROJECT_NAME}
    main.cpp
    runtime_tests.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE  aws-lambda-runtime GTest::GTest)
target_link_libraries(${PROJECT_NAME} PRIVATE aws-cpp-sdk-lambda aws-cpp-sdk-core curl)

gtest_discover_tests(${PROJECT_NAME})

if (GNU)
    # lambda must be built on Linux platforms
    add_executable(lambda_fun lambda_function.cpp)
    target_include_directories(lambda_fun PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(lambda_fun PRIVATE aws-lambda-runtime)
    # package the lambda function into lambda.zip and upload to s3
    add_custom_command(TARGET lambda_fun POST_BUILD 
        COMMAND "${CMAKE_SOURCE_DIR}/packaging/packager" "${CMAKE_CURRENT_BINARY_DIR}/lambda_fun" lambda_fun
        COMMAND aws s3 cp lambda_fun.zip s3://aws-lambda-cpp-runtime-tests/lambda_fun.zip
        )
endif()

