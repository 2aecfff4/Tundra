#include "loader/entry.h"
#include "core/std/assert.h"
#include "core/std/utils.h"

namespace tundra::vulkan_rhi::loader {

Entry::Entry() noexcept
{
#if TNDR_PLATFORM_WINDOWS
    const auto vulkan_lib = LoadLibraryA("vulkan-1.dll");
    if (vulkan_lib == nullptr) {
        core::panic("");
    }

    m_table.get_instance_proc_addr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(
        GetProcAddress(vulkan_lib, "vkGetInstanceProcAddr"));
#elif TNDR_PLATFORM_MAC || TNDR_PLATFORM_IOS
    auto* vulkan_lib = dlopen("libvulkan.dylib", RTLD_NOW | RTLD_LOCAL);
    if (vulkan_lib == nullptr) {
        vulkan_lib = dlopen("libvulkan.1.dylib", RTLD_NOW | RTLD_LOCAL);
    }
    if (vulkan_lib == nullptr) {
        vulkan_lib = dlopen("libMoltenVK.dylib", RTLD_NOW | RTLD_LOCAL);
    }
    if (vulkan_lib == nullptr) {
        core::panic("");
    }

    m_get_instance_proc_addr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(
        dlsym(vulkan_lib, "vkGetInstanceProcAddr"));
#else
    auto* vulkan_lib = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
    if (vulkan_lib == nullptr) {
        vulkan_lib = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
    }
    if (vulkan_lib == nullptr) {
        core::panic("");
    }

    m_get_instance_proc_addr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(
        dlsym(vulkan_lib, "vkGetInstanceProcAddr"));
#endif
    tndr_assert(
        m_table.get_instance_proc_addr != nullptr,
        "`get_instance_proc_addr` is nullptr!");

    const auto load =
        [f = m_table.get_instance_proc_addr](const char* name) -> PFN_vkVoidFunction {
        return f(nullptr, name);
    };

    // clang-format off
    m_table.create_instance = reinterpret_cast<PFN_vkCreateInstance>(load("vkCreateInstance"));
    m_table.enumerate_instance_layer_properties = reinterpret_cast<PFN_vkEnumerateInstanceLayerProperties>(load("vkEnumerateInstanceLayerProperties"));
    m_table.enumerate_instance_extension_properties = reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(load("vkEnumerateInstanceExtensionProperties"));
    m_table.enumerate_instance_version = reinterpret_cast<PFN_vkEnumerateInstanceVersion>(load("vkEnumerateInstanceVersion"));
    // clang-format on
}

core::Expected<Instance, VkResult> Entry::create_instance(
    const VkInstanceCreateInfo& create_info,
    const VkAllocationCallbacks* allocator) const noexcept
{
    VkInstance instance;
    const VkResult result = m_table.create_instance(&create_info, allocator, &instance);
    if (result == VK_SUCCESS) {
        return Instance(instance, m_table.get_instance_proc_addr);
    } else {
        return core::make_unexpected(result);
    }
}

core::Expected<core::Array<VkExtensionProperties>, VkResult>
    Entry::enumerate_instance_extension_properties() const noexcept
{
    u32 count = 0;
    VkResult result = m_table.enumerate_instance_extension_properties(
        nullptr, &count, nullptr);
    if (result != VK_SUCCESS) {
        return core::make_unexpected(result);
    }

    core::Array<VkExtensionProperties> extension_properties(count);
    result = m_table.enumerate_instance_extension_properties(
        nullptr, &count, extension_properties.data());
    if (result != VK_SUCCESS) {
        return core::make_unexpected(result);
    }

    return core::move(extension_properties);
}

core::Expected<core::Array<VkLayerProperties>, VkResult>
    Entry::enumerate_instance_layer_properties() const noexcept
{
    u32 count = 0;
    VkResult result = m_table.enumerate_instance_layer_properties(&count, nullptr);
    if (result != VK_SUCCESS) {
        return core::make_unexpected(result);
    }

    core::Array<VkLayerProperties> layer_properties(count);
    result = m_table.enumerate_instance_layer_properties(&count, layer_properties.data());
    if (result != VK_SUCCESS) {
        return core::make_unexpected(result);
    }

    return core::move(layer_properties);
}

const Entry::Table& Entry::get_table() const noexcept
{
    return m_table;
}

} // namespace tundra::vulkan_rhi::loader
