#pragma once
#include "core/core.h"
#include "core/std/containers/array.h"
#include "core/std/expected.h"
#include "loader/instance.h"
#include "vulkan_utils.h"

namespace tundra::vulkan_rhi::loader {

class Entry {
private:
    struct Table {
        // clang-format off
        PFN_vkGetInstanceProcAddr get_instance_proc_addr;
        PFN_vkCreateInstance create_instance;
        PFN_vkEnumerateInstanceExtensionProperties enumerate_instance_extension_properties;
        PFN_vkEnumerateInstanceLayerProperties enumerate_instance_layer_properties;
        PFN_vkEnumerateInstanceVersion enumerate_instance_version;
        // clang-format on
    } m_table;

public:
    Entry() noexcept;

public:
    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCreateInstance.html
    [[nodiscard]] core::Expected<Instance, VkResult> create_instance(
        const VkInstanceCreateInfo& create_info,
        const VkAllocationCallbacks* allocator) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkEnumerateInstanceExtensionProperties.html
    [[nodiscard]] core::Expected<core::Array<VkExtensionProperties>, VkResult>
        enumerate_instance_extension_properties() const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkEnumerateInstanceLayerProperties.html
    [[nodiscard]] core::Expected<core::Array<VkLayerProperties>, VkResult>
        enumerate_instance_layer_properties() const noexcept;

public:
    [[nodiscard]] const Table& get_table() const noexcept;
};

} // namespace tundra::vulkan_rhi::loader
