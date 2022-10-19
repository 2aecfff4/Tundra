#pragma once
#include "vulkan_utils.h"
#if defined(VK_KHR_android_surface)
#include "core/core.h"
#include "core/std/unexpected.h"
#include "loader/instance.h"

namespace tundra::vulkan_rhi::loader {
class Instance;
} // namespace tundra::vulkan_rhi::loader

namespace tundra::vulkan_rhi::loader::khr {

class AndroidSurface {
private:
    VkInstance m_instance;

    struct Table {
        // clang-format off
        PFN_vkCreateAndroidSurfaceKHR create_android_surface_khr;
        // clang-format on
    } m_table;

public:
    AndroidSurface(const loader::Instance& instance) noexcept;

public:
    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCreateAndroidSurfaceKHR.html
    [[nodiscard]] core::Unexpected<VkSurfaceKHR, VkResult> create_android_surface(
        const VkAndroidSurfaceCreateInfoKHR& create_info,
        const VkAllocationCallbacks* allocator) const noexcept;

public:
    [[nodiscard]] const Table& get_table() const noexcept;
    [[nodiscard]] static const char* name() noexcept;
};

} // namespace tundra::vulkan_rhi::loader::khr

#endif
