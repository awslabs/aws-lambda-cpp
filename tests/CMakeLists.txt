cmake_minimum_required(VERSION 3.11)
project(aws-lambda-runtime-tests LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(gtest
    URL https://github.com/google/googletest/archive/v1.12.0.tar.gz
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF)
FetchContent_MakeAvailable(gtest)

add_executable(unit_tests
    unit/thread_local_curl_test.cpp
    unit/unit_tests.cpp)
target_link_libraries(unit_tests PRIVATE gtest_main aws-lambda-runtime)

include(GoogleTest)
gtest_discover_tests(unit_tests
    PROPERTIES
    LABELS "unit"
    DISCOVERY_TIMEOUT 10)

add_subdirectory(resources)

