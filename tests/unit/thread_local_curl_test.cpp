#include <gtest/gtest.h>
#include <aws/lambda-runtime/runtime.h>
#include <thread>
#include <atomic>
#include <vector>

using namespace aws::lambda_runtime;

TEST(ThreadLocalCurl, runtime_construction_succeeds)
{
    runtime rt("http://127.0.0.1:9001");
    auto outcome = rt.get_next();
    // We expect a connection failure since there's no server, but the runtime itself should construct fine
    ASSERT_FALSE(outcome.is_success());
}

TEST(ThreadLocalCurl, multiple_runtimes_on_same_thread_do_not_crash)
{
    {
        runtime rt1("http://127.0.0.1:9001");
        auto o1 = rt1.get_next();
        ASSERT_FALSE(o1.is_success());
    }
    {
        runtime rt2("http://127.0.0.1:9001");
        auto o2 = rt2.get_next();
        ASSERT_FALSE(o2.is_success());
    }
}

TEST(ThreadLocalCurl, concurrent_runtimes_on_different_threads)
{
    constexpr int num_threads = 4;
    std::vector<std::thread> threads;
    std::atomic<int> successes{0};

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&successes]() {
            runtime rt("http://127.0.0.1:9001");
            auto outcome = rt.get_next();
            // Connection will fail but runtime should not crash or corrupt state
            if (!outcome.is_success()) {
                successes.fetch_add(1);
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    ASSERT_EQ(successes.load(), num_threads);
}

TEST(ThreadLocalCurl, sequential_requests_on_same_runtime)
{
    runtime rt("http://127.0.0.1:9001");
    for (int i = 0; i < 5; ++i) {
        auto outcome = rt.get_next();
        ASSERT_FALSE(outcome.is_success());
    }
}
