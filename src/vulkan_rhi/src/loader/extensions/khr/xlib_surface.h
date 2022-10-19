#pragma once
#include "vulkan_utils.h"

#if defined(VK_KHR_xlib_surface)
#include "core/core.h"
#include "core/std/expected.h"

namespace tundra::vulkan_rhi::loader {
class Instance;
} // namespace tundra::vulkan_rhi::loader

namespace tundra::vulkan_rhi::loader::khr {

class XLibSurface {
private:
    VkInstance m_instance;

    struct Table {
        // clang-format off
        PFN_vkCreateXlibSurfaceKHR create_xlib_surface_khr;
        PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR get_physical_device_xlib_presentation_support_khr;
        // clang-format on
    } m_table;

public:
    XLibSurface(const loader::Instance& instance) noexcept;

public:
    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCreateXlibSurfaceKHR.html
    [[nodiscard]] core::Expected<VkSurfaceKHR, VkResult> create_xlib_surface(
        const VkXlibSurfaceCreateInfoKHR& create_info,
        const VkAllocationCallbacks* allocator) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkGetPhysicalDeviceXlibPresentationSupportKHR.html
    [[nodiscard]] bool get_physical_device_xlib_presentation_support(
        const VkPhysicalDevice physical_device,
        const u32 queue_family_index,
        Display* dpy,
        VisualID visual_id) const noexcept;

public:
    [[nodiscard]] const Table& get_table() const noexcept;
    [[nodiscard]] static const char* name() noexcept;
};

} // namespace tundra::vulkan_rhi::loader::khr

#endif
