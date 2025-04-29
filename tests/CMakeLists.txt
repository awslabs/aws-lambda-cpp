cmake_minimum_required(VERSION 3.11)
project(aws-lambda-runtime-tests LANGUAGES CXX)

# Only fetch and setup Google Test in GitHub Actions
if(DEFINED ENV{GITHUB_RUN_ID})
    # Fetch Google Test for GitHub Actions
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

    # Common test library setup
    add_library(test_common INTERFACE)
    target_link_libraries(test_common INTERFACE GTest::GTest GTest::Main)

    # Unit tests executable
    add_executable(unit_tests
        unit/no_op_test.cpp)
    target_link_libraries(unit_tests PRIVATE test_common aws-lambda-runtime)

    # Register unit tests with CTest
    include(GoogleTest)
    gtest_discover_tests(unit_tests
        PROPERTIES
        LABELS "unit"
        DISCOVERY_TIMEOUT 10)
endif()

# Integration tests executable (only if enabled)
if (ENABLE_INTEGRATION_TESTS)
    find_package(AWSSDK COMPONENTS lambda iam)
    add_executable(integration_tests
        integration/main.cpp
        integration/runtime_tests.cpp
        integration/version_tests.cpp)
    target_link_libraries(integration_tests
        PRIVATE
        aws-lambda-runtime
        ${AWSSDK_LINK_LIBRARIES})

    # Register integration tests with CTest
    include(GoogleTest)
    gtest_discover_tests(integration_tests
        PROPERTIES
        LABELS "integration"
        EXTRA_ARGS "--aws_prefix=${TEST_RESOURCE_PREFIX}")

    add_subdirectory(resources)
endif()

