#pragma once
#include "core/std/containers/string.h"
#include "core/std/shared_ptr.h"
#include "rhi/resources/graphics_pipeline.h"
#include "vulkan_utils.h"

namespace tundra::vulkan_rhi {

class VulkanRawDevice;
struct Managers;
class VulkanPipelineLayoutManager;

///
class VulkanGraphicsPipeline {
private:
    core::SharedPtr<VulkanRawDevice> m_raw_device;
    VkPipeline m_pipeline;

public:
    VulkanGraphicsPipeline(
        core::SharedPtr<VulkanRawDevice> raw_device,
        const Managers& managers,
        const rhi::GraphicsPipelineCreateInfo& create_info) noexcept;
    ~VulkanGraphicsPipeline() noexcept;

    VulkanGraphicsPipeline(VulkanGraphicsPipeline&& rhs) noexcept;
    VulkanGraphicsPipeline& operator=(VulkanGraphicsPipeline&& rhs) noexcept;
    VulkanGraphicsPipeline(const VulkanGraphicsPipeline&) noexcept = delete;
    VulkanGraphicsPipeline& operator=(const VulkanGraphicsPipeline&) noexcept = delete;

public:
    [[nodiscard]] VkPipeline get_pipeline() const noexcept;
};

} // namespace tundra::vulkan_rhi
