project(aws-lambda-runtime-tests LANGUAGES CXX)
find_package(AWSSDK COMPONENTS lambda iam)
find_package(GTest REQUIRED)

add_executable(${PROJECT_NAME}
    main.cpp
    runtime_tests.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE ${AWSSDK_LINK_LIBRARIES} aws-lambda-runtime GTest::GTest)

# gtest_discover_tests(${PROJECT_NAME}) Can't use this until CMake 3.10 :(
# gtest_add_tests(TARGET ${PROJECT_NAME} runtime_tests.cpp)
GTEST_ADD_TESTS(${PROJECT_NAME} "" AUTO)

add_subdirectory(resources)

