#include <aws/core/Aws.h>
#include <gtest/gtest.h>
#include <aws/core/utils/logging/ConsoleLogSystem.h>

std::function<std::shared_ptr<Aws::Utils::Logging::LogSystemInterface>()> GetConsoleLoggerFactory()
{
    return [] {
        return Aws::MakeShared<Aws::Utils::Logging::ConsoleLogSystem>(
            "console_logger", Aws::Utils::Logging::LogLevel::Warn);
    };
}

int main(int argc, char** argv)
{
    Aws::SDKOptions options;
    options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Warn;
    options.loggingOptions.logger_create_fn = GetConsoleLoggerFactory();
    Aws::InitAPI(options);
    ::testing::InitGoogleTest(&argc, argv);
    int exitCode = RUN_ALL_TESTS();
    Aws::ShutdownAPI(options);
    return exitCode;
}

