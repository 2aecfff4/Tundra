#include "loader/extensions/khr/xcb_surface.h"

#if defined(VK_KHR_xcb_surface)
#include "loader/instance.h"

namespace tundra::vulkan_rhi::loader::khr {

XcbSurface::XcbSurface(const loader::Instance& instance) noexcept
    : m_instance(instance.get_handle())
{
    const auto load =
        [instance = m_instance,
         get_instance_proc_addr = instance.get_table().get_instance_proc_addr](
            const char* name) {
            return get_instance_proc_addr(instance, name);
        };

    // clang-format off
    m_table.create_xcb_surface_khr = reinterpret_cast<PFN_vkCreateXcbSurfaceKHR>(load("vkCreateXcbSurfaceKHR"));
    m_table.get_physical_device_xcb_presentation_support_khr = reinterpret_cast<PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR>(load("vkGetPhysicalDeviceXcbPresentationSupportKHR"));
    // clang-format on
}

core::Expected<VkSurfaceKHR, VkResult> XcbSurface::create_xcb_surface(
    const VkXcbSurfaceCreateInfoKHR& create_info,
    const VkAllocationCallbacks* allocator) const noexcept
{
    VkSurfaceKHR surface;
    const VkResult result = m_table.create_xcb_surface_khr(
        m_instance, &create_info, allocator, &surface);
    if (result == VK_SUCCESS) {
        return surface;
    } else {
        return core::make_unexpected(result);
    }
}

bool XcbSurface::get_physical_device_win32_presentation_support(
    const VkPhysicalDevice physical_device,
    const u32 queue_family_index,
    struct xcb_connection_t* connection,
    struct xcb_visualid_t visual_id) const noexcept
{
    const VkBool32 result = m_table.get_physical_device_xcb_presentation_support_khr(
        physical_device, queue_family_index, connection, visual_id);
    return static_cast<bool>(result);
}

const XcbSurface::Table& XcbSurface::get_table() const noexcept
{
    return m_table;
}

const char* XcbSurface::name() noexcept
{
    return VK_KHR_XCB_SURFACE_SPEC_VERSION;
}

} // namespace tundra::vulkan_rhi::loader::khr

#endif
