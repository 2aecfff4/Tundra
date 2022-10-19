#pragma once
#include "vulkan_utils.h"

#if defined(VK_KHR_xcb_surface)
#include "core/core.h"
#include "core/std/expected.h"

namespace tundra::vulkan_rhi::loader {
class Instance;
} // namespace tundra::vulkan_rhi::loader

namespace tundra::vulkan_rhi::loader::khr {

class XcbSurface {
private:
    VkInstance m_instance;

    struct Table {
        // clang-format off
        PFN_vkCreateXcbSurfaceKHR create_xcb_surface_khr;
        PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR get_physical_device_xcb_presentation_support_khr;
        // clang-format on
    } m_table;

public:
    XcbSurface(const loader::Instance& instance) noexcept;

public:
    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCreateXcbSurfaceKHR.html
    [[nodiscard]] core::Expected<VkSurfaceKHR, VkResult> create_xcb_surface(
        const VkXcbSurfaceCreateInfoKHR& create_info,
        const VkAllocationCallbacks* allocator) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkGetPhysicalDeviceXcbPresentationSupportKHR.html
    [[nodiscard]] bool get_physical_device_xcb_presentation_support(
        const VkPhysicalDevice physical_device,
        const u32 queue_family_index,
        struct xcb_connection_t* connection,
        struct xcb_visualid_t visual_id) const noexcept;

public:
    [[nodiscard]] const Table& get_table() const noexcept;
    [[nodiscard]] static const char* name() noexcept;
};

} // namespace tundra::vulkan_rhi::loader::khr

#endif
