#pragma once
#include "core/std/shared_ptr.h"
#include "rhi/config.h"
#include "vulkan_config.h"
#include "vulkan_utils.h"

namespace tundra::vulkan_rhi {

class VulkanRawDevice;

///
class VulkanPipelineCacheManager {
private:
    core::SharedPtr<VulkanRawDevice> m_raw_device;
    VkPipelineCache m_pipeline_cache;

public:
    VulkanPipelineCacheManager(core::SharedPtr<VulkanRawDevice> device) noexcept;
    ~VulkanPipelineCacheManager() noexcept;

public:
    [[nodiscard]] VkPipelineCache get_pipeline_cache() const noexcept;
};

} // namespace tundra::vulkan_rhi
