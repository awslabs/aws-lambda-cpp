project(lambda-test-fun LANGUAGES CXX)

# resources are the actual lambda functions deployed and exercised by the tests
add_executable(${PROJECT_NAME} lambda_function.cpp ${CMAKE_SOURCE_DIR}/src/backward.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PRIVATE aws-lambda-runtime)

find_package(Backtrace QUIET)
if(${Backtrace_FOUND})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${Backtrace_LIBRARIES})
    find_library(DW_LIB NAMES dw)
    if(NOT DW_LIB STREQUAL DW_LIB-NOTFOUND)
        target_compile_definitions(${PROJECT_NAME} PRIVATE "BACKWARD_HAS_DW=1")
        target_link_libraries(${PROJECT_NAME} PRIVATE "${DW_LIB}")
    endif()
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE "no_backtrace")
endif()
# package the lambda function into lambda.zip
add_custom_target(aws-lambda-package-lambda-test-fun
    COMMAND "${CMAKE_SOURCE_DIR}/packaging/packager" "${CMAKE_CURRENT_BINARY_DIR}/lambda-test-fun"
    DEPENDS ${PROJECT_NAME})

add_custom_target(aws-lambda-package-lambda-test-fun-no-glibc
    COMMAND "${CMAKE_SOURCE_DIR}/packaging/packager" -d "${CMAKE_CURRENT_BINARY_DIR}/lambda-test-fun"
    DEPENDS ${PROJECT_NAME})
