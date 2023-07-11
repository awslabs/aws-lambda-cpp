cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 11)

project(demo LANGUAGES CXX)

find_package(aws-lambda-runtime REQUIRED)
find_package(AWSSDK COMPONENTS core)
find_package(ZLIB)

add_executable(${PROJECT_NAME} "main.cpp")
target_link_libraries(${PROJECT_NAME} PUBLIC AWS::aws-lambda-runtime ${AWSSDK_LINK_LIBRARIES} )

# this line creates a target that packages your binary and zips it up
aws_lambda_package_target(${PROJECT_NAME} )
