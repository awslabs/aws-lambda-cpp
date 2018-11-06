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

#include "aws/lambda-runtime/runtime.h"
#include "aws/lambda-runtime/outcome.h"
#include "aws/lambda-runtime/logging.h"
#include "aws/http/response.h"

#include <curl/curl.h>
#include <climits>
#include <cassert>
#include <chrono>
#include <algorithm>
#include <cstdlib> // for strtoul

#define LAMBDA_RUNTIME_API __attribute__((visibility("default")))

namespace aws {
namespace lambda_runtime {

static const char LOG_TAG[] = "LAMBDA_RUNTIME";
static const char REQUEST_ID_HEADER[] = "lambda-runtime-aws-request-id";
static const char TRACE_ID_HEADER[] = "Lambda-Runtime-Trace-Id";
static const char CLIENT_CONTEXT_HEADER[] = "lambda-runtime-client-context";
static const char COGNITO_IDENTITY_HEADER[] = "lambda-runtime-cognito-identity";
static const char DEADLINE_MS_HEADER[] = "lambda-runtime-deadline-ms";
static const char FUNCTION_ARN_HEADER[] = "lambda-runtime-invoked-function-arn";

// static const char DEADLINE_NS[] = "X-Amz-Deadline-Ns";

enum Endpoints {
    INIT,
    NEXT,
    RESULT,
};

static bool is_success(aws::http::response_code httpcode)
{
    auto code = static_cast<int>(httpcode);
    return code >= 200 && code <= 299;
}

static size_t write_data(char* ptr, size_t size, size_t nmemb, void* userdata)
{
    if (!ptr) {
        return 0;
    }

    http::response* resp = static_cast<http::response*>(userdata);
    assert(size == 1);
    (void)size; // avoid warning in release builds
    assert(resp);
    resp->append_body(ptr, nmemb);
    return nmemb;
}

static inline std::string trim(std::string s)
{
    // trim right
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) { return !::isspace(ch); }).base(), s.end());
    // trim left
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) { return !::isspace(ch); }));
    return s;
}

static size_t write_header(char* ptr, size_t size, size_t nmemb, void* userdata)
{
    if (!ptr) {
        return 0;
    }

    logging::log_debug(LOG_TAG, "received header: %s", std::string(ptr, nmemb).c_str());

    http::response* resp = static_cast<http::response*>(userdata);
    for (size_t i = 0; i < nmemb; i++) {
        if (ptr[i] != ':')
            continue;
        std::string key{ptr, i};
        std::string value{ptr + i + 1, nmemb - i - 1};
        resp->add_header(trim(key), trim(value));
        break;
    }
    return size * nmemb;
}

static size_t read_data(char* buffer, size_t size, size_t nitems, void* userdata)
{
    const auto limit = size * nitems;
    auto ctx = static_cast<std::pair<std::string const&, size_t>*>(userdata);
    const auto unread = ctx->first.length() - ctx->second;
    if (0 == unread) {
        return 0;
    }

    if (unread <= limit) {
        std::copy_n(ctx->first.begin() + ctx->second, unread, buffer);
        ctx->second += unread;
        return unread;
    }

    std::copy_n(ctx->first.begin() + ctx->second, limit, buffer);
    ctx->second += limit;
    return limit;
}

struct no_result {
};

class runtime {
public:
    using get_next_outcome = aws::lambda_runtime::outcome<invocation_request, aws::http::response_code>;
    using post_result_outcome = aws::lambda_runtime::outcome<no_result, aws::http::response_code>;

    runtime(std::string const& endpoint);
    ~runtime();

    /**
     * Ask lambda for an invocation.
     */
    get_next_outcome get_next();

    /**
     * Tells lambda that the function has succeeded.
     */
    post_result_outcome post_success(std::string const& invocation_id, invocation_response const& response);

    /**
     * Tells lambda that the function has failed.
     */
    post_result_outcome post_failure(std::string const& invocation_id, invocation_response const& response);

private:
    void set_curl_next_options();
    void set_curl_post_result_options();
    post_result_outcome do_post(
        std::string const& url,
        std::string const& invocation_id,
        invocation_response const& response);

private:
    const std::array<std::string const, 3> m_endpoints;
    CURL* m_curl_handle;
};

runtime::runtime(std::string const& endpoint)
    : m_endpoints{{endpoint + "/2018-06-01/runtime/init/error",
                   endpoint + "/2018-06-01/runtime/invocation/next",
                   endpoint + "/2018-06-01/runtime/invocation/"}}
{
    m_curl_handle = curl_easy_init();
    if (!m_curl_handle) {
        logging::log_error(LOG_TAG, "Failed to acquire curl easy handle for next.");
    }
}

runtime::~runtime()
{
    curl_easy_cleanup(m_curl_handle);
}

void runtime::set_curl_next_options()
{
    // lambda freezes the container when no further tasks are available. The freezing period could be longer than the
    // request timeout, which causes the following get_next request to fail with a timeout error.
    curl_easy_reset(m_curl_handle);
    curl_easy_setopt(m_curl_handle, CURLOPT_TIMEOUT_MS, 0L);
    curl_easy_setopt(m_curl_handle, CURLOPT_CONNECTTIMEOUT_MS, 100L);
    curl_easy_setopt(m_curl_handle, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(m_curl_handle, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(m_curl_handle, CURLOPT_URL, m_endpoints[Endpoints::NEXT].c_str());

    curl_easy_setopt(m_curl_handle, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(m_curl_handle, CURLOPT_HEADERFUNCTION, write_header);
}

runtime::get_next_outcome runtime::get_next()
{

    http::response resp;
    set_curl_next_options();
    curl_easy_setopt(m_curl_handle, CURLOPT_WRITEDATA, &resp);
    curl_easy_setopt(m_curl_handle, CURLOPT_HEADERDATA, &resp);
    CURLcode curl_code = curl_easy_perform(m_curl_handle);

    if (curl_code != CURLE_OK) {
        logging::log_debug(LOG_TAG, "CURL returned error code %d - %s", curl_code, curl_easy_strerror(curl_code));
        logging::log_error(LOG_TAG, "Failed to get next invocation. No Response from endpoint");
        return aws::http::response_code::REQUEST_NOT_MADE;
    }

    {
        long resp_code;
        curl_easy_getinfo(m_curl_handle, CURLINFO_RESPONSE_CODE, &resp_code);
        resp.set_response_code(static_cast<aws::http::response_code>(resp_code));
    }

    {
        char* content_type = nullptr;
        curl_easy_getinfo(m_curl_handle, CURLINFO_CONTENT_TYPE, &content_type);
        resp.set_content_type(content_type);
    }

    if (!is_success(resp.get_response_code())) {
        logging::log_error(
            LOG_TAG,
            "Failed to get next invocation. Http Response code: %d",
            static_cast<int>(resp.get_response_code()));
        return resp.get_response_code();
    }

    if (!resp.has_header(REQUEST_ID_HEADER)) {
        logging::log_error(LOG_TAG, "Failed to find header %s in response", REQUEST_ID_HEADER);
        return aws::http::response_code::REQUEST_NOT_MADE;
    }
    invocation_request req;
    req.payload = resp.get_body();
    req.request_id = resp.get_header(REQUEST_ID_HEADER);
    req.xray_trace_id = resp.get_header(TRACE_ID_HEADER);
    req.client_context = resp.get_header(CLIENT_CONTEXT_HEADER);
    req.cognito_identity = resp.get_header(COGNITO_IDENTITY_HEADER);
    req.function_arn = resp.get_header(FUNCTION_ARN_HEADER);
    const auto deadline_string = resp.get_header(DEADLINE_MS_HEADER);
    unsigned long ms = strtoul(deadline_string.c_str(), nullptr, 10);
    assert(ms > 0);
    assert(ms < ULONG_MAX);
    req.deadline += std::chrono::milliseconds(ms);
    logging::log_info(
        LOG_TAG, "Received payload: %s\nTime remaining: %ld", req.payload.c_str(), req.get_time_remaining().count());
    return get_next_outcome(req);
}

#ifndef NDEBUG
static int rt_curl_debug_callback(CURL*, curl_infotype, char* data, size_t size, void*)
{
    std::string s(data, size);
    logging::log_debug(LOG_TAG, "CURL DBG: %s", s.c_str());
    return 0;
}
#endif

void runtime::set_curl_post_result_options()
{
    curl_easy_reset(m_curl_handle);
    curl_easy_setopt(m_curl_handle, CURLOPT_TIMEOUT_MS, 100L);
    curl_easy_setopt(m_curl_handle, CURLOPT_CONNECTTIMEOUT_MS, 100L);
    curl_easy_setopt(m_curl_handle, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(m_curl_handle, CURLOPT_POST, 1L);
    curl_easy_setopt(m_curl_handle, CURLOPT_READFUNCTION, read_data);
    curl_easy_setopt(m_curl_handle, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(m_curl_handle, CURLOPT_HEADERFUNCTION, write_header);

#ifndef NDEBUG
    curl_easy_setopt(m_curl_handle, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(m_curl_handle, CURLOPT_DEBUGFUNCTION, rt_curl_debug_callback);
#endif
}

runtime::post_result_outcome runtime::post_success(
    std::string const& request_id,
    invocation_response const& handler_response)
{
    const std::string url = m_endpoints[Endpoints::RESULT] + request_id + "/response";
    return do_post(url, request_id, handler_response);
}

runtime::post_result_outcome runtime::post_failure(
    std::string const& request_id,
    invocation_response const& handler_response)
{
    const std::string url = m_endpoints[Endpoints::RESULT] + request_id + "/error";
    return do_post(url, request_id, handler_response);
}

runtime::post_result_outcome runtime::do_post(
    std::string const& url,
    std::string const& request_id,
    invocation_response const& handler_response)
{
    set_curl_post_result_options();
    curl_easy_setopt(m_curl_handle, CURLOPT_URL, url.c_str());
    logging::log_info(LOG_TAG, "Making request to %s", url.c_str());

    curl_slist* headers = nullptr;
    if (handler_response.get_content_type().empty()) {
        headers = curl_slist_append(headers, "content-type: text/html");
    }
    else {
        headers = curl_slist_append(headers, ("content-type: " + handler_response.get_content_type()).c_str());
    }

    headers = curl_slist_append(headers, "Expect:");
    headers = curl_slist_append(headers, "transfer-encoding:");
    logging::log_debug(
        LOG_TAG,
        "calculating content length... %s",
        ("content-length: " + std::to_string(handler_response.get_payload().length())).c_str());
    headers = curl_slist_append(
        headers, ("content-length: " + std::to_string(handler_response.get_payload().length())).c_str());

    // set the body
    std::pair<std::string const&, size_t> ctx{handler_response.get_payload(), 0};
    aws::http::response resp;
    curl_easy_setopt(m_curl_handle, CURLOPT_WRITEDATA, &resp);
    curl_easy_setopt(m_curl_handle, CURLOPT_HEADERDATA, &resp);
    curl_easy_setopt(m_curl_handle, CURLOPT_READDATA, &ctx);
    curl_easy_setopt(m_curl_handle, CURLOPT_HTTPHEADER, headers);
    CURLcode curl_code = curl_easy_perform(m_curl_handle);
    curl_slist_free_all(headers);

    if (curl_code != CURLE_OK) {
        logging::log_debug(
            LOG_TAG,
            "CURL returned error code %d - %s, for invocation %s",
            curl_code,
            curl_easy_strerror(curl_code),
            request_id.c_str());
        return aws::http::response_code::REQUEST_NOT_MADE;
    }

    long http_response_code;
    curl_easy_getinfo(m_curl_handle, CURLINFO_RESPONSE_CODE, &http_response_code);

    if (!is_success(aws::http::response_code(http_response_code))) {
        logging::log_error(
            LOG_TAG, "Failed to post handler success response. Http response code: %ld.", http_response_code);
        return aws::http::response_code(http_response_code);
    }

    return post_result_outcome(no_result{});
}

static bool handle_post_outcome(runtime::post_result_outcome const& o, std::string const& request_id)
{
    if (o.is_success()) {
        return true;
    }

    if (o.get_failure() == aws::http::response_code::REQUEST_NOT_MADE) {
        logging::log_error(LOG_TAG, "Failed to send HTTP request for invocation %s.", request_id.c_str());
        return false;
    }

    logging::log_info(
        LOG_TAG,
        "HTTP Request for invocation %s was not successful. HTTP response code: %d.",
        request_id.c_str(),
        static_cast<int>(o.get_failure()));
    return false;
}

LAMBDA_RUNTIME_API void run_handler(std::function<invocation_response(invocation_request const&)> handler)
{
    logging::log_info(LOG_TAG, "Initializing the C++ Lambda Runtime.");
    std::string endpoint("http://");
    if (auto ep = std::getenv("AWS_LAMBDA_RUNTIME_API")) {
        puts(ep);
        assert(ep);
        logging::log_debug(LOG_TAG, "LAMBDA_SERVER_ADDRESS defined in environment as: %s", ep);
        endpoint += ep;
    }

    runtime rt(endpoint);

    while (true) {
        const auto next_outcome = rt.get_next();
        if (!next_outcome.is_success()) {
            if (next_outcome.get_failure() == aws::http::response_code::REQUEST_NOT_MADE) {
                logging::log_error(LOG_TAG, "Failed to send HTTP request to retrieve next task.");
                return;
            }

            logging::log_info(
                LOG_TAG,
                "HTTP request was not successful. HTTP response code: %d. Retrying..",
                static_cast<int>(next_outcome.get_failure()));
            continue;
        }

        const auto req = next_outcome.get_result();
        logging::log_info(LOG_TAG, "Invoking user handler");
        invocation_response res = handler(req);
        logging::log_info(LOG_TAG, "Invoking user handler completed.");

        if (res.is_success()) {
            const auto post_outcome = rt.post_success(req.request_id, res);
            if (!handle_post_outcome(post_outcome, req.request_id)) {
                return; // TODO: implement a better retry strategy
            }
        }
        else {
            const auto post_outcome = rt.post_failure(req.request_id, res);
            if (!handle_post_outcome(post_outcome, req.request_id)) {
                return; // TODO: implement a better retry strategy
            }
        }
    }
}

static std::string json_escape(std::string const& in)
{
    std::string out;
    out.reserve(in.length() * 2);
    for (size_t i = 0; i < in.length(); i++) {
        if ((in[i] > 31) && in[i] != '\"' && (in[i] != '\\')) {
            out.append(1, in[i]);
        }
        else {
            out.append(1, '\\');
            switch (in[i]) {
                case '\\':
                    out.append(1, '\\');
                    break;
                case '\"':
                    out.append(1, '\"');
                    break;
                case '\b':
                    out.append(1, 'b');
                    break;
                case '\f':
                    out.append(1, 'f');
                    break;
                case '\n':
                    out.append(1, 'n');
                    break;
                case '\r':
                    out.append(1, 'r');
                    break;
                case '\t':
                    out.append(1, 't');
                    break;
                default:
                    /* escape and print as unicode codepoint */
                    char buf[6];
                    sprintf(buf, "u%04x", in[i]);
                    out.append(buf, 5);
                    break;
            }
        }
    }
    return out;
}

LAMBDA_RUNTIME_API invocation_response invocation_response::success(std::string const& payload, std::string const& content_type)
{
    invocation_response r;
    r.m_success = true;
    r.m_content_type = content_type;
    r.m_payload = payload;
    return r;
}

LAMBDA_RUNTIME_API invocation_response invocation_response::failure(std::string const& error_message, std::string const& error_type)
{
    invocation_response r;
    r.m_success = false;
    r.m_content_type = "application/json";
    r.m_payload = "{\"errorMessage\":\"" + json_escape(error_message) +
                  "\","
                  "\"errorType\":\"" +
                  json_escape(error_type) +
                  "\","
                  "\"stackTrace\": null}";
    return r;
}

} // namespace lambda_runtime
} // namespace aws
