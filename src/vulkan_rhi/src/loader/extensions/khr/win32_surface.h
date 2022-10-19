#pragma once
#include "vulkan_utils.h"

#if defined(VK_KHR_win32_surface)
#include "core/core.h"
#include "core/std/expected.h"

namespace tundra::vulkan_rhi::loader {
class Instance;
} // namespace tundra::vulkan_rhi::loader

namespace tundra::vulkan_rhi::loader::khr {

class Win32Surface {
private:
    VkInstance m_instance;

    struct Table {
        // clang-format off
        PFN_vkCreateWin32SurfaceKHR create_win32_surface_khr;
        PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR get_physical_device_win32_presentation_support_khr;
        // clang-format on
    } m_table;

public:
    Win32Surface(const loader::Instance& instance) noexcept;

public:
    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCreateWin32SurfaceKHR.html
    [[nodiscard]] core::Expected<VkSurfaceKHR, VkResult> create_win32_surface(
        const VkWin32SurfaceCreateInfoKHR& create_info,
        const VkAllocationCallbacks* allocator) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkGetPhysicalDeviceWin32PresentationSupportKHR.html
    [[nodiscard]] bool get_physical_device_win32_presentation_support(
        const VkPhysicalDevice physical_device,
        const u32 queue_family_index) const noexcept;

public:
    [[nodiscard]] const Table& get_table() const noexcept;
    [[nodiscard]] static const char* name() noexcept;
};

} // namespace tundra::vulkan_rhi::loader::khr

#endif
