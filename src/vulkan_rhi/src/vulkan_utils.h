#pragma once
#include "core/std/expected.h"
#include "core/std/panic.h"

// Windows stuff
#if TNDR_PLATFORM_WINDOWS
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#endif // PLATFORM_WINDOWS

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

namespace tundra::vulkan_rhi {

[[nodiscard]] const char* vk_result_to_str(const VkResult result) noexcept;

template <typename Exp>
auto vulkan_map_result(Exp&& exp, const char* msg) noexcept
{
    using T = std::decay_t<Exp>;

    if (!exp) {
        core::panic("{}. Error: `{}`", msg, vk_result_to_str(exp.error()));
    }

    if constexpr (!std::is_void_v<typename T::ValueType>) {
        return *core::forward<Exp>(exp);
    }
}

} // namespace tundra::vulkan_rhi
