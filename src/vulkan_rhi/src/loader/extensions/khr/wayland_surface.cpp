#include "loader/extensions/khr/wayland_surface.h"

#if defined(VK_KHR_wayland_surface)
#include "loader/instance.h"

namespace tundra::vulkan_rhi::loader::khr {

WaylandSurface::WaylandSurface(const loader::Instance& instance) noexcept
    : m_instance(instance.get_handle())
{
    const auto load =
        [instance = m_instance,
         get_instance_proc_addr = instance.get_table().get_instance_proc_addr](
            const char* name) {
            return get_instance_proc_addr(instance, name);
        };

    // clang-format off
    m_table.create_wayland_surface_khr = reinterpret_cast<PFN_vkCreateWaylandSurfaceKHR>(load("vkCreateWaylandSurfaceKHR"));
    m_table.get_physical_device_wayland_presentation_support_khr = reinterpret_cast<PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR>(load("vkGetPhysicalDeviceWaylandPresentationSupportKHR"));
    // clang-format on
}

core::Expected<VkSurfaceKHR, VkResult> WaylandSurface::create_wayland_surface(
    const VkWaylandSurfaceCreateInfoKHR& create_info,
    const VkAllocationCallbacks* allocator) const noexcept
{
    VkSurfaceKHR surface;
    const VkResult result = m_table.create_wayland_surface_khr(
        m_instance, &create_info, allocator, &surface);
    if (result == VK_SUCCESS) {
        return surface;
    } else {
        return core::make_unexpected(result);
    }
}

bool WaylandSurface::get_physical_device_wayland_presentation_support(
    const VkPhysicalDevice physical_device,
    const u32 queue_family_index,
    const struct wl_display* display) const noexcept
{
    const VkBool32 result = m_table.get_physical_device_wayland_presentation_support_khr(
        physical_device, queue_family_index, display);
    return static_cast<bool>(result);
}

const WaylandSurface::Table& WaylandSurface::get_table() const noexcept
{
    return m_table;
}

const char* WaylandSurface::name() noexcept
{
    return VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME;
}

} // namespace tundra::vulkan_rhi::loader::khr

#endif
