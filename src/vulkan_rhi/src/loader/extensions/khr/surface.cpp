#include "loader/extensions/khr/surface.h"

#if defined(VK_KHR_surface)
#include "loader/instance.h"

namespace tundra::vulkan_rhi::loader::khr {

Surface::Surface(const loader::Instance& instance) noexcept
    : m_instance(instance.get_handle())
{
    const auto load =
        [instance = m_instance,
         get_instance_proc_addr = instance.get_table().get_instance_proc_addr](
            const char* name) {
            return get_instance_proc_addr(instance, name);
        };

    // clang-format off
    m_table.destroy_surface_khr = reinterpret_cast<PFN_vkDestroySurfaceKHR>(load("vkDestroySurfaceKHR"));
    m_table.get_physical_device_surface_capabilities_khr = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>(load("vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
    m_table.get_physical_device_surface_formats_khr = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR>(load("vkGetPhysicalDeviceSurfaceFormatsKHR"));
    m_table.get_physical_device_surface_present_modes_khr = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR>(load("vkGetPhysicalDeviceSurfacePresentModesKHR"));
    m_table.get_physical_device_surface_support_khr = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceSupportKHR>(load("vkGetPhysicalDeviceSurfaceSupportKHR"));
    // clang-format on
}

void Surface::destroy_surface(
    const VkSurfaceKHR surface, const VkAllocationCallbacks* allocator) const noexcept
{
    m_table.destroy_surface_khr(m_instance, surface, allocator);
}

core::Expected<VkSurfaceCapabilitiesKHR, VkResult>
    Surface::get_physical_device_surface_capabilities(
        const VkPhysicalDevice physical_device, const VkSurfaceKHR surface) const noexcept
{
    VkSurfaceCapabilitiesKHR surface_capabilities;
    const VkResult result = m_table.get_physical_device_surface_capabilities_khr(
        physical_device, surface, &surface_capabilities);

    if (result != VK_SUCCESS) {
        return core::make_unexpected(result);
    } else {
        return surface_capabilities;
    }
}

core::Expected<core::Array<VkSurfaceFormatKHR>, VkResult>
    Surface::get_physical_device_surface_formats(
        const VkPhysicalDevice physical_device, const VkSurfaceKHR surface) const noexcept
{
    u32 num = 0;
    VkResult result = m_table.get_physical_device_surface_formats_khr(
        physical_device, surface, &num, nullptr);
    if (result != VK_SUCCESS) {
        return core::make_unexpected(result);
    }

    core::Array<VkSurfaceFormatKHR> surface_formats(num);
    result = m_table.get_physical_device_surface_formats_khr(
        physical_device, surface, &num, surface_formats.data());

    if (result != VK_SUCCESS) {
        return core::make_unexpected(result);
    }

    return core::move(surface_formats);
}

core::Expected<core::Array<VkPresentModeKHR>, VkResult>
    Surface::get_physical_device_surface_present_modes(
        const VkPhysicalDevice physical_device, const VkSurfaceKHR surface) const noexcept
{
    u32 num = 0;
    VkResult result = m_table.get_physical_device_surface_present_modes_khr(
        physical_device, surface, &num, nullptr);
    if (result != VK_SUCCESS) {
        return core::make_unexpected(result);
    }

    core::Array<VkPresentModeKHR> present_modes(num);
    result = m_table.get_physical_device_surface_present_modes_khr(
        physical_device, surface, &num, present_modes.data());

    if (result != VK_SUCCESS) {
        return core::make_unexpected(result);
    }

    return core::move(present_modes);
}

core::Expected<bool, VkResult> Surface::get_physical_device_surface_support(
    const VkPhysicalDevice physical_device,
    const u32 queue_family_index,
    const VkSurfaceKHR surface) const noexcept
{
    VkBool32 is_supported;
    const VkResult result = m_table.get_physical_device_surface_support_khr(
        physical_device, queue_family_index, surface, &is_supported);

    if (result != VK_SUCCESS) {
        return core::make_unexpected(result);
    } else {
        return static_cast<bool>(is_supported);
    }
}

const Surface::Table& Surface::get_table() const noexcept
{
    return m_table;
}

const char* Surface::name() noexcept
{
    return VK_KHR_SURFACE_EXTENSION_NAME;
}

} // namespace tundra::vulkan_rhi::loader::khr

#endif
