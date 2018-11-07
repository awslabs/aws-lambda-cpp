project(aws-lambda-runtime-resources LANGUAGES CXX)

add_executable(lambda_fun lambda_function.cpp)
target_include_directories(lambda_fun PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(lambda_fun PRIVATE aws-lambda-runtime)
# package the lambda function into lambda.zip and upload to s3
add_custom_command(TARGET lambda_fun POST_BUILD
    COMMAND "${CMAKE_SOURCE_DIR}/packaging/packager" "${CMAKE_CURRENT_BINARY_DIR}/lambda_fun" "output"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

