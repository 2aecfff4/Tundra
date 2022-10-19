#include "loader/extensions/khr/win32_surface.h"

#if defined(VK_KHR_win32_surface)
#include "loader/instance.h"

namespace tundra::vulkan_rhi::loader::khr {

Win32Surface::Win32Surface(const loader::Instance& instance) noexcept
    : m_instance(instance.get_handle())
{
    const auto load =
        [instance = m_instance,
         get_instance_proc_addr = instance.get_table().get_instance_proc_addr](
            const char* name) {
            return get_instance_proc_addr(instance, name);
        };

    // clang-format off
    m_table.create_win32_surface_khr = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(load("vkCreateWin32SurfaceKHR"));
    m_table.get_physical_device_win32_presentation_support_khr = reinterpret_cast<PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR>(load("vkGetPhysicalDeviceWin32PresentationSupportKHR"));
    // clang-format on
}

core::Expected<VkSurfaceKHR, VkResult> Win32Surface::create_win32_surface(
    const VkWin32SurfaceCreateInfoKHR& create_info,
    const VkAllocationCallbacks* allocator) const noexcept
{
    VkSurfaceKHR surface;
    const VkResult result = m_table.create_win32_surface_khr(
        m_instance, &create_info, allocator, &surface);
    if (result == VK_SUCCESS) {
        return surface;
    } else {
        return core::make_unexpected(result);
    }
}

bool Win32Surface::get_physical_device_win32_presentation_support(
    const VkPhysicalDevice physical_device, const u32 queue_family_index) const noexcept
{
    const VkBool32 result = m_table.get_physical_device_win32_presentation_support_khr(
        physical_device, queue_family_index);
    return static_cast<bool>(result);
}

const Win32Surface::Table& Win32Surface::get_table() const noexcept
{
    return m_table;
}

const char* Win32Surface::name() noexcept
{
    return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
}

} // namespace tundra::vulkan_rhi::loader::khr

#endif
