#pragma once
#include "core/core.h"
#include "core/std/containers/array.h"
#include "core/std/shared_ptr.h"
#include "loader/entry.h"
#include "vulkan_utils.h"

namespace tundra::vulkan_rhi {

class VulkanDevice;
class VulkanInstance;

class VulkanContext {
private:
    loader::Entry m_entry;
    core::SharedPtr<VulkanInstance> m_instance;
    core::Array<core::SharedPtr<VulkanDevice>> m_devices;

public:
    struct Device {
        VkPhysicalDevice physical_device;
        VkPhysicalDeviceType device_type;

        struct Queues {
            u32 graphics;
            u32 compute;
            u32 transfer;
        } queues;
    };

public:
    VulkanContext() noexcept;

private:
    void create_device() noexcept;

private:
    [[nodiscard]] core::Array<VulkanContext::Device> enumerate_devices() noexcept;

public:
    [[nodiscard]] const core::SharedPtr<VulkanDevice>& get_device() const noexcept;
};

} // namespace tundra::vulkan_rhi
