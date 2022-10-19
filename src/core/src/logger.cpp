#include "core/logger.h"
#include "core/std/panic.h"
#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <filesystem>

#if TNDR_PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif // TNDR_PLATFORM_WINDOWS

namespace tundra::core {

Logger::Logger() noexcept
{
#if TNDR_PLATFORM_WINDOWS
    const HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (!(handle == INVALID_HANDLE_VALUE)) {
        DWORD mode;
        if (GetConsoleMode(handle, &mode)) {
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(handle, mode);
        }
    }
#endif // TNDR_PLATFORM_WINDOWS
}

core::Logger& Logger::get() noexcept
{
    static Logger s_logger;
    return s_logger;
}

void Logger::log_private(
    const LogVerbosity verbosity,
    const std::source_location& source_location,
    const std::string& msg) noexcept
{
    const fmt::color color = [&] {
        switch (verbosity) {
            case LogVerbosity::Error:
                return fmt::color::red;
            case LogVerbosity::Warn:
                return fmt::color::orange;
            case LogVerbosity::Info:
                return fmt::color::light_gray;
            default:
                core::panic("Invalid enum!");
        }
    }();

    const std::time_t time = std::time(nullptr);
#if TNDR_PLATFORM_WINDOWS
    tm result;
    localtime_s(&result, &time);
    const auto* lc = &result;
#else
    const tm* const lc = std::localtime(&time);
#endif

    const auto filename = [&] {
        char separators[] = {
            std::filesystem::path::preferred_separator,
            '/',
            '\0',
        };
        const std::string_view path = source_location.file_name();
        const auto pos = path.find_last_of(separators);
        return (pos == std::string_view::npos) ? path : path.substr(pos + 1);
    }();

    fmt::print(
        fmt::fg(color),
        "[{:04d}.{:02d}.{:02d} {:02d}:{:02d}:{:02d}]:[{}({}, {})]: {}\n",
        lc->tm_year + 1900,
        lc->tm_mon + 1,
        lc->tm_mday,
        lc->tm_hour,
        lc->tm_min,
        lc->tm_sec,
        filename,
        source_location.line(),
        source_location.column(),
        msg //
    );
}

} // namespace tundra::core
