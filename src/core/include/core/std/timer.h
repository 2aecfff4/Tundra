#pragma once
#include "core/core_export.h"
#include "core/core.h"
#include <chrono>

namespace tundra::core {

///
class CORE_API Timer {
public:
    Timer() noexcept;

public:
    void tick() noexcept;
    void reset() noexcept;

public:
    [[nodiscard]] f32 get_delta_time() const noexcept;
    [[nodiscard]] f32 get_lifetime() const noexcept;

private:
    std::chrono::high_resolution_clock::time_point m_epoch;
    std::chrono::high_resolution_clock::time_point m_previous_time;
};

} // namespace tundra::core
