#pragma once
#include <string>
#include <spdlog/spdlog.h>

namespace kengine {
    class Logger {
    public:
        virtual ~Logger() = default;

        virtual void logInfo(const std::string& message) = 0;
        virtual void logWarning(const std::string& message) = 0;
        virtual void logError(const std::string& message) = 0;
    };

    class SpdlogLogger : public Logger {
    public:
        void logInfo(const std::string& message) override {
            spdlog::info(message);
        }

        void logWarning(const std::string& message) override {
            spdlog::warn(message);
        }

        void logError(const std::string& message) override {
            spdlog::error(message);
        }
    };

    class LogManager {
    public:
        static Logger& getLogger() {
            static SpdlogLogger logger;
            return logger;
        }
    };
}