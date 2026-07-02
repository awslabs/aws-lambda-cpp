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
        unit/thread_local_curl_test.cpp
        unit/unit_tests.cpp)
    target_link_libraries(unit_tests PRIVATE gtest_main aws-lambda-runtime)

    # Register unit tests
    include(GoogleTest)
    gtest_discover_tests(unit_tests
        PROPERTIES
        LABELS "unit"
        DISCOVERY_TIMEOUT 10)
else()
    # Build unit tests using the bundled gtest
    add_executable(unit_tests
        unit/thread_local_curl_test.cpp
        unit/unit_tests.cpp
        gtest/gtest-all.cc)
    target_include_directories(unit_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(unit_tests PRIVATE aws-lambda-runtime pthread)

    # Provide a main() for gtest
    target_sources(unit_tests PRIVATE unit/main.cpp)

    include(GoogleTest)
    gtest_discover_tests(unit_tests
        PROPERTIES
        LABELS "unit"
        DISCOVERY_TIMEOUT 10)
endif()
FetchContent_Declare(gtest
    URL https://github.com/google/googletest/archive/v1.12.0.tar.gz
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF)
FetchContent_MakeAvailable(gtest)

add_executable(unit_tests
    unit/no_op_test.cpp)
target_link_libraries(unit_tests PRIVATE gtest_main aws-lambda-runtime)

include(GoogleTest)
gtest_discover_tests(unit_tests
    PROPERTIES
    LABELS "unit"
    DISCOVERY_TIMEOUT 10)


add_subdirectory(resources)

