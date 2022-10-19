#pragma once
#include "core/core.h"
#include "core/std/shared_ptr.h"
#include "rhi/config.h"
#include "vulkan_config.h"
#include "vulkan_utils.h"

namespace tundra::vulkan_rhi {

class VulkanRawDevice;

///
struct PipelineLayout {
    VkPipelineLayout pipeline_layout;
    VkDescriptorSetLayout descriptor_layouts[NUM_BINDINGS];
};

///
class VulkanPipelineLayoutManager {
private:
    core::SharedPtr<VulkanRawDevice> m_raw_device;
    PipelineLayout m_pipeline_layout;

public:
    VulkanPipelineLayoutManager(core::SharedPtr<VulkanRawDevice> raw_device) noexcept;
    ~VulkanPipelineLayoutManager() noexcept;

public:
    [[nodiscard]] const PipelineLayout& get_pipeline_layout() const noexcept;
};

} // namespace tundra::vulkan_rhi
