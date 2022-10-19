#pragma once
#include "vulkan_utils.h"

#if defined(VK_KHR_wayland_surface)
#include "core/core.h"
#include "core/std/expected.h"

namespace tundra::vulkan_rhi::loader {
class Instance;
} // namespace tundra::vulkan_rhi::loader

namespace tundra::vulkan_rhi::loader::khr {

class WaylandSurface {
private:
    VkInstance m_instance;

    struct Table {
        // clang-format off
        PFN_vkCreateWaylandSurfaceKHR create_wayland_surface_khr;
        PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR get_physical_device_wayland_presentation_support_khr;
        // clang-format on
    } m_table;

public:
    WaylandSurface(const loader::Instance& instance) noexcept;

public:
    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCreateWaylandSurfaceKHR.html
    [[nodiscard]] core::Expected<VkSurfaceKHR, VkResult> create_wayland_surface(
        const VkWaylandSurfaceCreateInfoKHR& create_info,
        const VkAllocationCallbacks* allocator) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkGetPhysicalDeviceWaylandPresentationSupportKHR.html
    [[nodiscard]] bool get_physical_device_wayland_presentation_support(
        const VkPhysicalDevice physical_device,
        const u32 queue_family_index,
        const struct wl_display* display) const noexcept;

public:
    [[nodiscard]] const Table& get_table() const noexcept;
    [[nodiscard]] static const char* name() noexcept;
};

} // namespace tundra::vulkan_rhi::loader::khr

#endif
