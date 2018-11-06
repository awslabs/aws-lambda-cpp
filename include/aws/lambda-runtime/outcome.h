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

#include <cassert>

namespace aws {
namespace lambda_runtime {

template <typename TResult, typename TFailure>
class outcome {
public:
    outcome(TResult const& s) : s(s), success(true) {}

    outcome(TFailure const& f) : f(f), success(false) {}

    outcome(outcome&& other) : success(other.success)
    {
        if (success) {
            s = std::move(other.s);
        }
        else {
            f = std::move(other.f);
        }
    }

    ~outcome()
    {
        if (success) {
            s.~TResult();
        }
        else {
            f.~TFailure();
        }
    }

    TResult const& get_result() const
    {
        assert(success);
        return s;
    }

    TFailure const& get_failure() const
    {
        assert(!success);
        return f;
    }

    bool is_success() const { return success; }

private:
    union {
        TResult s;
        TFailure f;
    };
    bool success;
};
} // namespace lambda_runtime
} // namespace aws
