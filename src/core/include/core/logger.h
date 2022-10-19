#pragma once
#include "core/core_export.h"
#include "core/core.h"
#include "core/std/utils.h"
#include <fmt/format.h>
#include <source_location>

namespace tundra::core {

///
enum class LogVerbosity : u8 {
    Info,
    Warn,
    Error,
};

///
class CORE_API Logger {
public:
    Logger(Logger&&) noexcept = delete;
    Logger& operator=(Logger&&) noexcept = delete;
    Logger(const Logger&) noexcept = delete;
    Logger& operator=(const Logger&) noexcept = delete;

private:
    Logger() noexcept;
    ~Logger() noexcept = default;

public:
    [[nodiscard]] static Logger& get() noexcept;

private:
    void log_private(
        const LogVerbosity verbosity,
        const std::source_location& source_location,
        const std::string& msg) noexcept;

public:
    template <typename... Args>
    void log(
        const LogVerbosity verbosity,
        const std::source_location& source_location,
        const char* fmt,
        Args&&... args) noexcept
    {
        this->log_private(
            verbosity, source_location, fmt::format(fmt, core::forward<Args>(args)...));
    }
};

} // namespace tundra::core

///
#define tndr_info(format, ...)                                                           \
    tundra::core::Logger::get().log(                                                     \
        tundra::core::LogVerbosity::Info,                                                \
        std::source_location::current(),                                                 \
        format,                                                                          \
        __VA_ARGS__)

///
#define tndr_warn(format, ...)                                                           \
    tundra::core::Logger::get().log(                                                     \
        tundra::core::LogVerbosity::Warn,                                                \
        std::source_location::current(),                                                 \
        format,                                                                          \
        __VA_ARGS__)

///
#define tndr_error(format, ...)                                                          \
    tundra::core::Logger::get().log(                                                     \
        tundra::core::LogVerbosity::Error,                                               \
        std::source_location::current(),                                                 \
        format,                                                                          \
        __VA_ARGS__)
