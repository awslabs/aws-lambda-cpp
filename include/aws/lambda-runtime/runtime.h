#pragma once
/*
 * Copyright 2018-present Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include <chrono>
#include <string>
#include <functional>

namespace aws {
namespace lambda_runtime {

struct invocation_request {
    /**
     * The user's payload represented as a UTF-8 string.
     */
    std::string payload;

    /**
     * An identifier unique to the current invocation.
     */
    std::string request_id;

    /**
     * X-Ray tracing ID of the current invocation.
     */
    std::string xray_trace_id;

    /**
     * Information about the client application and device when invoked through the AWS Mobile SDK.
     */
    std::string client_context;

    /**
     * Information about the Amazon Cognito identity provider when invoked through the AWS Mobile SDK.
     */
    std::string cognito_identity;

    /**
     * The ARN requested. This can be different in each invoke that executes the same version.
     */
    std::string function_arn;

    /**
     * Function execution deadline counted in milliseconds since the Unix epoch.
     */
    std::chrono::time_point<std::chrono::system_clock> deadline;

    /**
     * The number of milliseconds left before lambda terminates the current execution.
     */
    inline std::chrono::milliseconds get_time_remaining() const;
};

class invocation_response {
private:
    /**
     * The output of the function which is sent to the lambda caller.
     */
    std::string m_payload;

    /**
     * The MIME type of the payload.
     * This is always set to 'application/json' in unsuccessful invocations.
     */
    std::string m_content_type;

    /**
     * Flag to distinguish if the contents are for successful or unsuccessful invocations.
     */
    bool m_success;

    /**
     * Instantiate an empty response. Used by the static functions 'success' and 'failure' to create a populated
     * invocation_response
     */
    invocation_response() = default;

public:
    /**
     * Create a successful invocation response with the given payload and content-type.
     */
    static invocation_response success(std::string const& payload, std::string const& content_type);

    /**
     * Create a failure response with the given error message and error type.
     * The content-type is always set to application/json in this case.
     */
    static invocation_response failure(std::string const& error_message, std::string const& error_type);

    /**
     * Get the MIME type of the payload.
     */
    std::string const& get_content_type() const { return m_content_type; }

    /**
     * Get the payload string. The string is assumed to be UTF-8 encoded.
     */
    std::string const& get_payload() const { return m_payload; }

    /**
     * Returns true if the payload and content-type are set. Returns false if the error message and error types are set.
     */
    bool is_success() const { return m_success; }
};

inline std::chrono::milliseconds invocation_request::get_time_remaining() const
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(deadline - system_clock::now());
}

// Entry method
void run_handler(std::function<invocation_response(invocation_request const&)> const& handler);

} // namespace lambda_runtime
} // namespace aws
