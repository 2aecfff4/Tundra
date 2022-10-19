#pragma once
#include "vulkan_utils.h"

#if defined(VK_KHR_surface)
#include "core/core.h"
#include "core/std/containers/array.h"
#include "core/std/expected.h"

namespace tundra::vulkan_rhi::loader {
class Instance;
} // namespace tundra::vulkan_rhi::loader

namespace tundra::vulkan_rhi::loader::khr {

class Surface {
private:
    VkInstance m_instance;

    struct Table {
        // clang-format off
        PFN_vkDestroySurfaceKHR destroy_surface_khr;
        PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR get_physical_device_surface_capabilities_khr;
        PFN_vkGetPhysicalDeviceSurfaceFormatsKHR get_physical_device_surface_formats_khr;
        PFN_vkGetPhysicalDeviceSurfacePresentModesKHR get_physical_device_surface_present_modes_khr;
        PFN_vkGetPhysicalDeviceSurfaceSupportKHR get_physical_device_surface_support_khr;
        // clang-format on
    } m_table;

public:
    Surface(const loader::Instance& instance) noexcept;

public:
    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkDestroySurfaceKHR.html
    void destroy_surface(
        const VkSurfaceKHR surface,
        const VkAllocationCallbacks* allocator) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkGetPhysicalDeviceSurfaceCapabilitiesKHR.html
    [[nodiscard]] core::Expected<VkSurfaceCapabilitiesKHR, VkResult>
        get_physical_device_surface_capabilities(
            const VkPhysicalDevice physical_device,
            const VkSurfaceKHR surface) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkGetPhysicalDeviceSurfaceFormatsKHR.html
    [[nodiscard]] core::Expected<core::Array<VkSurfaceFormatKHR>, VkResult>
        get_physical_device_surface_formats(
            const VkPhysicalDevice physical_device,
            const VkSurfaceKHR surface) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkGetPhysicalDeviceSurfacePresentModesKHR.html
    [[nodiscard]] core::Expected<core::Array<VkPresentModeKHR>, VkResult>
        get_physical_device_surface_present_modes(
            const VkPhysicalDevice physical_device,
            const VkSurfaceKHR surface) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkGetPhysicalDeviceSurfaceSupportKHR.html
    [[nodiscard]] core::Expected<bool, VkResult> get_physical_device_surface_support(
        const VkPhysicalDevice physical_device,
        const u32 queue_family_index,
        const VkSurfaceKHR surface) const noexcept;

public:
    [[nodiscard]] const Table& get_table() const noexcept;
    [[nodiscard]] static const char* name() noexcept;
};

} // namespace tundra::vulkan_rhi::loader::khr

#endif
