#include "loader/extensions/khr/xlib_surface.h"

#if defined(VK_KHR_xlib_surface)
#include "loader/instance.h"

namespace tundra::vulkan_rhi::loader::khr {

XLibSurface::XLibSurface(const loader::Instance& instance) noexcept
    : m_instance(instance.get_handle())
{
    const auto load =
        [instance = m_instance,
         get_instance_proc_addr = instance.get_table().get_instance_proc_addr](
            const char* name) {
            return get_instance_proc_addr(instance, name);
        };

    // clang-format off
    m_table.create_xlib_surface_khr = reinterpret_cast<PFN_vkCreateXlibSurfaceKHR>(load("vkCreateXlibSurfaceKHR"));
    m_table.get_physical_device_xlib_presentation_support_khr = reinterpret_cast<PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR>(load("vkGetPhysicalDeviceXlibPresentationSupportKHR"));
    // clang-format on
}

core::Expected<VkSurfaceKHR, VkResult> XLibSurface::create_xlib_surface(
    const VkXlibSurfaceCreateInfoKHR& create_info,
    const VkAllocationCallbacks* allocator) const noexcept
{
    VkSurfaceKHR surface;
    const VkResult result = m_table.create_xlib_surface_khr(
        m_instance, &create_info, allocator, &surface);
    if (result == VK_SUCCESS) {
        return surface;
    } else {
        return core::make_unexpected(result);
    }
}

bool XLibSurface::get_physical_device_xlib_presentation_support(
    const VkPhysicalDevice physical_device,
    const u32 queue_family_index,
    Display* dpy,
    VisualID visual_id) const noexcept
{
    const VkBool32 result = m_table.get_physical_device_xlib_presentation_support_khr(
        physical_device, queue_family_index, dpy, visual_id);
    return static_cast<bool>(result);
}

const XLibSurface::Table& XLibSurface::get_table() const noexcept
{
    return m_table;
}

const char* XLibSurface::name() noexcept
{
    return VK_KHR_XLIB_SURFACE_SPEC_VERSION;
}

} // namespace tundra::vulkan_rhi::loader::khr

#endif
