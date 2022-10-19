#pragma once
#include "core/core.h"
#include "core/std/shared_ptr.h"
#include "rhi/resources/handle.h"
#include "rhi/resources/handle_manager.h"

namespace tundra::rhi {
class ResourceTracker;
}

namespace tundra::vulkan_rhi {

class VulkanSwapchain;
class VulkanBuffer;
class VulkanTexture;
class VulkanShader;
class VulkanGraphicsPipeline;
class VulkanComputePipeline;
class VulkanSampler;
class VulkanTextureView;

class VulkanRawDevice;
class VulkanPipelineLayoutManager;
class VulkanPipelineCacheManager;
class VulkanCommandBufferManager;
class VulkanDescriptorBindlessManager;
class VulkanFramebufferManager;
class VulkanPipelineManager;
class VulkanRenderPassManager;

struct Managers {
    core::SharedPtr<rhi::HandleManager<rhi::SwapchainHandleType, VulkanSwapchain>>
        swapchain_manager;
    core::SharedPtr<rhi::HandleManager<rhi::BufferHandleType, VulkanBuffer>>
        buffer_manager;
    core::SharedPtr<rhi::HandleManager<rhi::TextureHandleType, VulkanTexture>>
        texture_manager;
    core::SharedPtr<rhi::HandleManager<rhi::TextureViewHandleType, VulkanTextureView>>
        texture_view_manager;
    core::SharedPtr<rhi::HandleManager<rhi::ShaderHandleType, VulkanShader>>
        shader_manager;
    core::SharedPtr<
        rhi::HandleManager<rhi::GraphicsPipelineHandleType, VulkanGraphicsPipeline>>
        graphics_pipeline_manager;
    core::SharedPtr<
        rhi::HandleManager<rhi::ComputePipelineHandleType, VulkanComputePipeline>>
        compute_pipeline_manager;
    core::SharedPtr<rhi::HandleManager<rhi::SamplerHandleType, VulkanSampler>>
        sampler_manager;
    core::SharedPtr<rhi::ResourceTracker> resource_tracker;
    core::SharedPtr<VulkanPipelineLayoutManager> pipeline_layout_manager;
    core::SharedPtr<VulkanPipelineCacheManager> pipeline_cache_manager;
    core::SharedPtr<VulkanCommandBufferManager> command_buffer_manager;
    core::SharedPtr<VulkanDescriptorBindlessManager> descriptor_bindless_manager;

public:
    Managers(core::SharedPtr<VulkanRawDevice> device) noexcept;
    Managers(const Managers&) noexcept = default;
    Managers& operator=(const Managers&) noexcept = default;
    Managers(Managers&&) noexcept = default;
    Managers& operator=(Managers&&) noexcept = default;
};

} // namespace tundra::vulkan_rhi
