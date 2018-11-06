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
