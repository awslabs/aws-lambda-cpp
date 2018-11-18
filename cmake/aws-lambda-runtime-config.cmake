include(${CMAKE_CURRENT_LIST_DIR}/@CMAKE_PROJECT_NAME@-targets.cmake)

set(AWS_LAMBDA_PACKAGING_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/packager)
function(aws_lambda_package_target target)
    add_custom_target(aws-lambda-package-${target}
        COMMAND ${AWS_LAMBDA_PACKAGING_SCRIPT} $<TARGET_FILE:${target}>
        DEPENDS ${target})
endfunction()

