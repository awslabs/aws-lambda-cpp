function(aws_lambda_package_target target)
    add_custom_target(NAME aws-lambda-package
        COMMAND
        ${CMAKE_CURRENT_LIST_DIR}/packager $<TARGET_FILE:${target}> ${target})
endfunction()
