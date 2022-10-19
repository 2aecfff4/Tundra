#pragma once
#include "core/std/shared_ptr.h"
#include "rhi/resources/compute_pipeline.h"
#include "vulkan_utils.h"

namespace tundra::vulkan_rhi {

class VulkanRawDevice;
struct Managers;

class VulkanComputePipeline {
private:
    core::SharedPtr<VulkanRawDevice> m_raw_device;
    VkPipeline m_pipeline;

public:
    VulkanComputePipeline(
        core::SharedPtr<VulkanRawDevice> raw_device,
        const Managers& managers,
        const rhi::ComputePipelineCreateInfo& create_info) noexcept;
    ~VulkanComputePipeline() noexcept;

    VulkanComputePipeline(VulkanComputePipeline&& rhs) noexcept;
    VulkanComputePipeline& operator=(VulkanComputePipeline&& rhs) noexcept;
    VulkanComputePipeline(const VulkanComputePipeline&) noexcept = delete;
    VulkanComputePipeline& operator=(const VulkanComputePipeline&) noexcept = delete;

public:
    [[nodiscard]] VkPipeline get_pipeline() const noexcept;
};

} // namespace tundra::vulkan_rhi
