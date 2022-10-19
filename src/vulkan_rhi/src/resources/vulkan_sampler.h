#pragma once
#include "core/core.h"
#include "core/std/shared_ptr.h"
#include "rhi/resources/sampler.h"
#include "vulkan_utils.h"

namespace tundra::vulkan_rhi {

class VulkanRawDevice;
struct DeviceLimits;

///
class VulkanSampler {
private:
    core::SharedPtr<VulkanRawDevice> m_raw_device;
    VkSampler m_sampler;

public:
    VulkanSampler(
        core::SharedPtr<VulkanRawDevice> raw_device,
        const rhi::SamplerCreateInfo& create_info) noexcept;
    ~VulkanSampler() noexcept;

    VulkanSampler(VulkanSampler&& rhs) noexcept;
    VulkanSampler& operator=(VulkanSampler&& rhs) noexcept;
    VulkanSampler(const VulkanSampler&) noexcept = delete;
    VulkanSampler& operator=(const VulkanSampler&) noexcept = delete;

public:
    [[nodiscard]] VkSampler get_sampler() const noexcept;
};

} // namespace tundra::vulkan_rhi
