project(aws-lambda-runtime-tests LANGUAGES CXX)
find_package(AWSSDK COMPONENTS core lambda)
find_package(GTest REQUIRED)

add_executable(${PROJECT_NAME}
    main.cpp
    runtime_tests.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE  ${AWSSDK_LINK_LIBRARIES} aws-lambda-runtime GTest::GTest)

gtest_discover_tests(${PROJECT_NAME})

if (GNU)
	add_subdirectory(resources)
endif()

