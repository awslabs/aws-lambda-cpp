project(aws-lambda-runtime-tests LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(gtest
  QUIET
  URL https://github.com/google/googletest/archive/v1.15.2.tar.gz
)
# configure build of googletest
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF)
FetchContent_MakeAvailable(gtest)

add_executable(
    unit_tests
    no_op_test.cpp
    )

target_link_libraries(unit_tests
  PRIVATE
    gtest_main
  )

# automatic discovery of unit tests
include(GoogleTest)
gtest_discover_tests(unit_tests
  PROPERTIES
    LABELS "unit"
  DISCOVERY_TIMEOUT  # how long to wait (in seconds) before crashing
    240
  )
