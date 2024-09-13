#pragma once
#include <string>
#include <spdlog/spdlog.h>

namespace kengine {
    class Logger {
    public:
        virtual ~Logger() = default;

        virtual void info(const std::string& message) = 0;
        virtual void warning(const std::string& message) = 0;
        virtual void error(const std::string& message) = 0;
    };

    class SpdlogLogger : public Logger {
    public:
        void info(const std::string& message) override {
            spdlog::info(message);
        }

        void warning(const std::string& message) override {
            spdlog::warn(message);
        }

        void error(const std::string& message) override {
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

#define KE_LOG_LEVEL_TRACE 0
#define KE_LOG_LEVEL_DEBUG 1
#define KE_LOG_LEVEL_INFO 2
#define KE_LOG_LEVEL_WARN 3
#define KE_LOG_LEVEL_ERROR 4
#define KE_LOG_LEVEL_OFF 5

// Preprocessor directive to allow for overriding the log level at compile time
#ifndef KE_ACTIVE_LOG_LEVEL
#define KE_ACTIVE_LOG_LEVEL KE_LOG_LEVEL_INFO
#endif

#define KENGINE_LOGGER() kengine::LogManager::getLogger()

#if KE_ACTIVE_LOG_LEVEL <= KE_LOG_LEVEL_TRACE
#define KE_LOG_TRACE(...) KENGINE_LOGGER().trace(__VA_ARGS__)
#else
#define KE_LOG_TRACE(...) (void)0
#endif

#if KE_ACTIVE_LOG_LEVEL <= KE_LOG_LEVEL_DEBUG
#define KE_LOG_DEBUG(...) KENGINE_LOGGER().debug(__VA_ARGS__)
#else
#define KE_LOG_DEBUG(...) (void)0
#endif

#if KE_ACTIVE_LOG_LEVEL <= KE_LOG_LEVEL_INFO
#define KE_LOG_INFO(...) KENGINE_LOGGER().info(__VA_ARGS__)
#else
#define KE_LOG_INFO(...) (void)0
#endif

#if KE_ACTIVE_LOG_LEVEL <= KE_LOG_LEVEL_WARN
#define KE_LOG_WARN(...) KENGINE_LOGGER().warn(__VA_ARGS__)
#else
#define KE_LOG_WARN(...) (void)0
#endif

#if KE_ACTIVE_LOG_LEVEL <= KE_LOG_LEVEL_ERROR
#define KE_LOG_ERROR(...) KENGINE_LOGGER().error(__VA_ARGS__)
#else
#define KE_LOG_ERROR(...) (void)0 
#endif