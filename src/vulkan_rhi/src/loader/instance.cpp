#include "loader/instance.h"

namespace tundra::vulkan_rhi::loader {

Instance::Instance(
    const VkInstance instance, PFN_vkGetInstanceProcAddr get_instance_proc_addr) noexcept
    : m_instance(instance)
{
    const auto load = [instance, get_instance_proc_addr](const char* name) {
        return get_instance_proc_addr(instance, name);
    };

    // clang-format off
    m_table.get_instance_proc_addr = get_instance_proc_addr;
    m_table.create_device = reinterpret_cast<PFN_vkCreateDevice>(load("vkCreateDevice"));
    m_table.destroy_instance = reinterpret_cast<PFN_vkDestroyInstance>(load("vkDestroyInstance"));
    m_table.enumerate_device_extension_properties = reinterpret_cast<PFN_vkEnumerateDeviceExtensionProperties>(load("vkEnumerateDeviceExtensionProperties"));
    m_table.enumerate_device_layer_properties = reinterpret_cast<PFN_vkEnumerateDeviceLayerProperties>(load("vkEnumerateDeviceLayerProperties"));
    m_table.enumerate_physical_devices = reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(load("vkEnumeratePhysicalDevices"));
    m_table.get_device_proc_addr = reinterpret_cast<PFN_vkGetDeviceProcAddr>(load("vkGetDeviceProcAddr"));
    m_table.get_physical_device_features = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures>(load("vkGetPhysicalDeviceFeatures"));
    m_table.get_physical_device_format_properties = reinterpret_cast<PFN_vkGetPhysicalDeviceFormatProperties>(load("vkGetPhysicalDeviceFormatProperties"));
    m_table.get_physical_device_image_format_properties = reinterpret_cast<PFN_vkGetPhysicalDeviceImageFormatProperties>(load("vkGetPhysicalDeviceImageFormatProperties"));
    m_table.get_physical_device_memory_properties = reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties>(load("vkGetPhysicalDeviceMemoryProperties"));
    m_table.get_physical_device_properties = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(load("vkGetPhysicalDeviceProperties"));
    m_table.get_physical_device_queue_family_properties = reinterpret_cast<PFN_vkGetPhysicalDeviceQueueFamilyProperties>(load("vkGetPhysicalDeviceQueueFamilyProperties"));
    m_table.get_physical_device_sparse_image_format_properties = reinterpret_cast<PFN_vkGetPhysicalDeviceSparseImageFormatProperties>(load("vkGetPhysicalDeviceSparseImageFormatProperties"));
    m_table.enumerate_physical_device_groups = reinterpret_cast<PFN_vkEnumeratePhysicalDeviceGroups>(load("vkEnumeratePhysicalDeviceGroups"));
    m_table.get_physical_device_external_buffer_properties = reinterpret_cast<PFN_vkGetPhysicalDeviceExternalBufferProperties>(load("vkGetPhysicalDeviceExternalBufferProperties"));
    m_table.get_physical_device_external_fence_properties = reinterpret_cast<PFN_vkGetPhysicalDeviceExternalFenceProperties>(load("vkGetPhysicalDeviceExternalFenceProperties"));
    m_table.get_physical_device_external_semaphore_properties = reinterpret_cast<PFN_vkGetPhysicalDeviceExternalSemaphoreProperties>(load("vkGetPhysicalDeviceExternalSemaphoreProperties"));
    m_table.get_physical_device_features2 = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2>(load("vkGetPhysicalDeviceFeatures2"));
    m_table.get_physical_device_format_properties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceFormatProperties2>(load("vkGetPhysicalDeviceFormatProperties2"));
    m_table.get_physical_device_image_format_properties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceImageFormatProperties2>(load("vkGetPhysicalDeviceImageFormatProperties2"));
    m_table.get_physical_device_memory_properties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties2>(load("vkGetPhysicalDeviceMemoryProperties2"));
    m_table.get_physical_device_properties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2>(load("vkGetPhysicalDeviceProperties2"));
    m_table.get_physical_device_queue_family_properties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceQueueFamilyProperties2>(load("vkGetPhysicalDeviceQueueFamilyProperties2"));
    m_table.get_physical_device_sparse_image_format_properties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceSparseImageFormatProperties2>(load("vkGetPhysicalDeviceSparseImageFormatProperties2"));
    // clang-format on
}

core::Expected<Device, VkResult> Instance::create_device(
    const VkPhysicalDevice physical_device,
    const VkDeviceCreateInfo& create_info,
    const VkAllocationCallbacks* allocator) const noexcept
{
    VkDevice device;
    const VkResult result = m_table.create_device(
        physical_device, &create_info, allocator, &device);
    if (result == VK_SUCCESS) {
        return Device(device, m_table.get_device_proc_addr);
    } else {
        return core::make_unexpected(result);
    }
}

void Instance::destroy_instance(const VkAllocationCallbacks* allocator) noexcept
{
    m_table.destroy_instance(m_instance, allocator);
    m_instance = VK_NULL_HANDLE;
}

core::Expected<core::Array<VkExtensionProperties>, VkResult>
    Instance::enumerate_device_extension_properties(
        const VkPhysicalDevice physical_device) const noexcept
{
    u32 num = 0;
    VkResult result = m_table.enumerate_device_extension_properties(
        physical_device, nullptr, &num, nullptr);
    if (result != VK_SUCCESS) {
        return core::make_unexpected(result);
    }

    core::Array<VkExtensionProperties> extension_properties(num);
    result = m_table.enumerate_device_extension_properties(
        physical_device, nullptr, &num, extension_properties.data());

    if (result != VK_SUCCESS) {
        return core::make_unexpected(result);
    }

    return core::move(extension_properties);
}

core::Expected<core::Array<VkPhysicalDevice>, VkResult>
    Instance::enumerate_physical_devices() const noexcept
{
    u32 num = 0;
    VkResult result = m_table.enumerate_physical_devices(m_instance, &num, nullptr);
    if (result != VK_SUCCESS) {
        return core::make_unexpected(result);
    }

    core::Array<VkPhysicalDevice> physical_devices(num);
    result = m_table.enumerate_physical_devices(
        m_instance, &num, physical_devices.data());

    if (result != VK_SUCCESS) {
        return core::make_unexpected(result);
    }

    return core::move(physical_devices);
}

void Instance::get_physical_device_features2(
    const VkPhysicalDevice physical_device,
    VkPhysicalDeviceFeatures2& features) const noexcept
{
    m_table.get_physical_device_features2(physical_device, &features);
}

void Instance::get_physical_device_properties2(
    const VkPhysicalDevice physical_device,
    VkPhysicalDeviceProperties2& properties) const noexcept
{
    m_table.get_physical_device_properties2(physical_device, &properties);
}

core::Array<VkQueueFamilyProperties> Instance::get_physical_device_queue_family_properties(
    const VkPhysicalDevice physical_device) const noexcept
{
    u32 queue_count = 0;
    m_table.get_physical_device_queue_family_properties(
        physical_device, &queue_count, nullptr);

    core::Array<VkQueueFamilyProperties> queue_families(queue_count);
    m_table.get_physical_device_queue_family_properties(
        physical_device, &queue_count, queue_families.data());

    return core::move(queue_families);
}

VkInstance Instance::get_handle() const noexcept
{
    return m_instance;
}

const Instance::Table& Instance::get_table() const noexcept
{
    return m_table;
}

} // namespace tundra::vulkan_rhi::loader
