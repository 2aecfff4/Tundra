#include "managers/managers.h"
#include "managers/vulkan_command_buffer_manager.h"
#include "managers/vulkan_descriptor_bindless_manager.h"
#include "managers/vulkan_pipeline_cache_manager.h"
#include "managers/vulkan_pipeline_layout_manager.h"
#include "resources/vulkan_buffer.h"
#include "resources/vulkan_compute_pipeline.h"
#include "resources/vulkan_graphics_pipeline.h"
#include "resources/vulkan_sampler.h"
#include "resources/vulkan_shader.h"
#include "resources/vulkan_swapchain.h"
#include "resources/vulkan_texture.h"
#include "resources/vulkan_texture_view.h"

namespace tundra::vulkan_rhi {

Managers::Managers(core::SharedPtr<VulkanRawDevice> device) noexcept
    : swapchain_manager(
          core::make_shared<rhi::HandleManager<rhi::SwapchainHandleType, VulkanSwapchain>>(
              "Swapchain"))
    , buffer_manager(
          core::make_shared<rhi::HandleManager<rhi::BufferHandleType, VulkanBuffer>>(
              "Buffer"))
    , texture_manager(
          core::make_shared<rhi::HandleManager<rhi::TextureHandleType, VulkanTexture>>(
              "Texture"))
    , texture_view_manager(
          core::make_shared<
              rhi::HandleManager<rhi::TextureViewHandleType, VulkanTextureView>>(
              "TextureView"))
    , shader_manager(
          core::make_shared<rhi::HandleManager<rhi::ShaderHandleType, VulkanShader>>(
              "Shader"))
    , graphics_pipeline_manager(
          core::make_shared<
              rhi::HandleManager<rhi::GraphicsPipelineHandleType, VulkanGraphicsPipeline>>(
              "GraphicsPipeline"))
    , compute_pipeline_manager(
          core::make_shared<
              rhi::HandleManager<rhi::ComputePipelineHandleType, VulkanComputePipeline>>(
              "ComputePipeline"))
    , sampler_manager(
          core::make_shared<rhi::HandleManager<rhi::SamplerHandleType, VulkanSampler>>(
              "SamplerManager"))
    , resource_tracker(core::make_shared<rhi::ResourceTracker>())
    , pipeline_layout_manager(core::make_shared<VulkanPipelineLayoutManager>(device))
    , pipeline_cache_manager(core::make_shared<VulkanPipelineCacheManager>(device))
    , command_buffer_manager(
          core::make_shared<VulkanCommandBufferManager>(device, resource_tracker))
    , descriptor_bindless_manager(core::make_shared<VulkanDescriptorBindlessManager>(
          device, pipeline_layout_manager))
{
}

} // namespace tundra::vulkan_rhi
