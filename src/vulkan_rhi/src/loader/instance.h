#pragma once
#include "core/core.h"
#include "core/std/containers/array.h"
#include "core/std/expected.h"
#include "loader/device.h"
#include "vulkan_utils.h"

namespace tundra::vulkan_rhi::loader {

class Instance {
private:
    VkInstance m_instance;
    struct Table {
        // clang-format off
        PFN_vkGetInstanceProcAddr get_instance_proc_addr;
        PFN_vkCreateDevice create_device;
        PFN_vkDestroyInstance destroy_instance;
        PFN_vkEnumerateDeviceExtensionProperties enumerate_device_extension_properties;
        PFN_vkEnumerateDeviceLayerProperties enumerate_device_layer_properties;
        PFN_vkEnumeratePhysicalDevices enumerate_physical_devices;
        PFN_vkGetDeviceProcAddr get_device_proc_addr;
        PFN_vkGetPhysicalDeviceFeatures get_physical_device_features;
        PFN_vkGetPhysicalDeviceFormatProperties get_physical_device_format_properties;
        PFN_vkGetPhysicalDeviceImageFormatProperties get_physical_device_image_format_properties;
        PFN_vkGetPhysicalDeviceMemoryProperties get_physical_device_memory_properties;
        PFN_vkGetPhysicalDeviceProperties get_physical_device_properties;
        PFN_vkGetPhysicalDeviceQueueFamilyProperties get_physical_device_queue_family_properties;
        PFN_vkGetPhysicalDeviceSparseImageFormatProperties get_physical_device_sparse_image_format_properties;
        PFN_vkEnumeratePhysicalDeviceGroups enumerate_physical_device_groups;
        PFN_vkGetPhysicalDeviceExternalBufferProperties get_physical_device_external_buffer_properties;
        PFN_vkGetPhysicalDeviceExternalFenceProperties get_physical_device_external_fence_properties;
        PFN_vkGetPhysicalDeviceExternalSemaphoreProperties get_physical_device_external_semaphore_properties;
        PFN_vkGetPhysicalDeviceFeatures2 get_physical_device_features2;
        PFN_vkGetPhysicalDeviceFormatProperties2 get_physical_device_format_properties2;
        PFN_vkGetPhysicalDeviceImageFormatProperties2 get_physical_device_image_format_properties2;
        PFN_vkGetPhysicalDeviceMemoryProperties2 get_physical_device_memory_properties2;
        PFN_vkGetPhysicalDeviceProperties2 get_physical_device_properties2;
        PFN_vkGetPhysicalDeviceQueueFamilyProperties2 get_physical_device_queue_family_properties2;
        PFN_vkGetPhysicalDeviceSparseImageFormatProperties2 get_physical_device_sparse_image_format_properties2;
        // clang-format on
    } m_table;

private:
    Instance(
        const VkInstance instance,
        PFN_vkGetInstanceProcAddr get_instance_proc_addr) noexcept;

public:
    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCreateDevice.html
    [[nodiscard]] core::Expected<Device, VkResult> create_device(
        const VkPhysicalDevice physical_device,
        const VkDeviceCreateInfo& create_info,
        const VkAllocationCallbacks* allocator) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkDestroyInstance.html
    void destroy_instance(const VkAllocationCallbacks* allocator) noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkEnumerateDeviceExtensionProperties.html
    [[nodiscard]] core::Expected<core::Array<VkExtensionProperties>, VkResult>
        enumerate_device_extension_properties(
            const VkPhysicalDevice physical_device) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkEnumeratePhysicalDevices.html
    [[nodiscard]] core::Expected<core::Array<VkPhysicalDevice>, VkResult>
        enumerate_physical_devices() const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkGetPhysicalDeviceFeatures2.html
    void get_physical_device_features2(
        const VkPhysicalDevice physical_device,
        VkPhysicalDeviceFeatures2& features) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkGetPhysicalDeviceProperties2.html
    void get_physical_device_properties2(
        const VkPhysicalDevice physical_device,
        VkPhysicalDeviceProperties2& properties) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkGetPhysicalDeviceQueueFamilyProperties2.html
    [[nodiscard]] core::Array<VkQueueFamilyProperties>
        get_physical_device_queue_family_properties(
            const VkPhysicalDevice physical_device) const noexcept;

public:
    [[nodiscard]] VkInstance get_handle() const noexcept;
    [[nodiscard]] const Table& get_table() const noexcept;

private:
    friend class Entry;
};

} // namespace tundra::vulkan_rhi::loader
