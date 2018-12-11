cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 11)
project(aws-lambda-runtime
    VERSION 0.1.0
    LANGUAGES CXX)

option(ENABLE_TESTS "Enables building the test project, requires AWS C++ SDK." OFF)

include(CheckCXXSourceCompiles)
include(CheckCXXCompilerFlag)

check_cxx_source_compiles("
#include <execinfo.h>

int main() {
void *buffer[5];
int nptrs = backtrace(buffer, 5);
char **strings = backtrace_symbols(buffer, nptrs);

return !!strings;
}"  HAVE_BACKTRACE)


check_cxx_compiler_flag("-Wl,-flto" LTO_CAPABLE)

add_library(${PROJECT_NAME}
    "src/logging.cpp"
    "src/runtime.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/version.cpp"
    )

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

find_package(CURL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ${CURL_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CURL_INCLUDE_DIRS})

if(HAVE_BACKTRACE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DHAVE_BACKTRACE)
endif()

target_compile_options(${PROJECT_NAME} PRIVATE
    "-fno-exceptions"
    "-fno-rtti"
    "-fvisibility=hidden"
    "-Wall"
    "-Wextra"
    "-Werror"
    "-Wconversion"
    "-Wno-sign-conversion")

if (LOG_VERBOSITY)
    target_compile_definitions(${PROJECT_NAME} PRIVATE "AWS_LAMBDA_LOG=${LOG_VERBOSITY}")
elseif(CMAKE_BUILD_TYPE STREQUAL Debug)
    target_compile_definitions(${PROJECT_NAME} PRIVATE "AWS_LAMBDA_LOG=3")
else ()
    target_compile_definitions(${PROJECT_NAME} PRIVATE "AWS_LAMBDA_LOG=0")
endif()

if ((BUILD_SHARED_LIBS) AND (LTO_CAPABLE))
    target_compile_options(${PROJECT_NAME} PRIVATE "-flto")
    target_link_libraries(${PROJECT_NAME} PRIVATE "-flto")
endif()

#tests
if (ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

#versioning
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/version.cpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/version.cpp"
    NEWLINE_STYLE LF)

# installation
install(FILES "include/aws/lambda-runtime/runtime.h" "include/aws/lambda-runtime/version.h"
    DESTINATION "include/aws/lambda-runtime")

install(FILES "include/aws/logging/logging.h"
    DESTINATION "include/aws/logging")

install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

configure_file("${CMAKE_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
    @ONLY)

export(EXPORT "${PROJECT_NAME}-targets" NAMESPACE AWS::)

install(EXPORT "${PROJECT_NAME}-targets"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/${PROJECT_NAME}/cmake/"
    NAMESPACE AWS::)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/${PROJECT_NAME}/cmake/")

install(PROGRAMS "${CMAKE_SOURCE_DIR}/packaging/packager"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/${PROJECT_NAME}/cmake/")

