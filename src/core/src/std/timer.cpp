#include "core/std/timer.h"

namespace tundra::core {

Timer::Timer() noexcept
    : m_epoch(std::chrono::high_resolution_clock::now())
    , m_previous_time(std::chrono::high_resolution_clock::now())
{
}

void Timer::tick() noexcept
{
    m_previous_time = std::chrono::high_resolution_clock::now();
}

void Timer::reset() noexcept
{
    m_epoch = std::chrono::high_resolution_clock::now();
    m_previous_time = std::chrono::high_resolution_clock::now();
}

f32 Timer::get_delta_time() const noexcept
{
    const auto now = std::chrono::high_resolution_clock::now();
    const auto duration =
        std::chrono::duration_cast<std::chrono::duration<f32, std::micro>>(
            now - m_previous_time);
    const std::chrono::duration<f32, std::deci> delta = now - m_previous_time;
    return duration.count();
}

f32 Timer::get_lifetime() const noexcept
{
    const auto now = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<f32> lifetime = now - m_epoch;
    return lifetime.count();
}

} // namespace tundra::core
