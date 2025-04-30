cmake_minimum_required(VERSION 3.11)
project(aws-lambda-runtime-tests LANGUAGES CXX)

if(DEFINED ENV{GITHUB_ACTIONS})
    # Fetch Google Test for unit tests
    include(FetchContent)
    FetchContent_Declare(gtest
        URL https://github.com/google/googletest/archive/v1.12.0.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    # Configure build of googletest
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF)
    FetchContent_MakeAvailable(gtest)

    add_executable(unit_tests
        unit/no_op_test.cpp)
    target_link_libraries(unit_tests PRIVATE gtest_main aws-lambda-runtime)

    # Register unit tests
    include(GoogleTest)
    gtest_discover_tests(unit_tests
        PROPERTIES
        LABELS "unit"
        DISCOVERY_TIMEOUT 10)
else()
    message(STATUS "Unit tests skipped: Not in GitHub Actions environment")
endif()


find_package(AWSSDK COMPONENTS lambda iam QUIET)

if(AWSSDK_FOUND)
    add_executable(${PROJECT_NAME}
        integration/main.cpp
        integration/runtime_tests.cpp
        integration/version_tests.cpp
        gtest/gtest-all.cc)

    target_link_libraries(${PROJECT_NAME} PRIVATE ${AWSSDK_LINK_LIBRARIES} aws-lambda-runtime)

    include(GoogleTest)
    gtest_discover_tests(${PROJECT_NAME} EXTRA_ARGS "--aws_prefix=${TEST_RESOURCE_PREFIX}")

    add_subdirectory(resources)
else()
    message(STATUS "Integration tests skipped: AWS SDK not found or not in GitHub Actions environment")
endif()

