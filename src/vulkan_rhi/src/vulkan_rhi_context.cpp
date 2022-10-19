#include "vulkan_rhi_context.h"
#include "core/profiler.h"
#include "vulkan_device.h"

namespace tundra::vulkan_rhi {

const char* VulkanRHIContext::get_name() const noexcept
{
    return "vulkan_rhi";
}

rhi::GraphicsAPI VulkanRHIContext::get_graphics_api() const noexcept
{
    return rhi::GraphicsAPI::Vulkan;
}

rhi::QueueFamilyIndices VulkanRHIContext::get_queue_family_indices() const noexcept
{
    const VulkanQueues& device_queues = m_vulkan_context.get_device()->get_queues();
    return rhi::QueueFamilyIndices {
        .graphics_queue = core::get<u32>(device_queues.graphics_queue),
        .compute_queue = core::get<u32>(device_queues.compute_queue),
        .transfer_queue = core::get<u32>(device_queues.transfer_queue),
        .present_queue = core::get<u32>(device_queues.present_queue),
    };
}

void VulkanRHIContext::submit(
    core::Array<rhi::SubmitInfo> submit_infos,
    core::Array<rhi::PresentInfo> present_infos) noexcept
{
    TNDR_PROFILER_TRACE("VulkanRHIContext::submit");

    m_vulkan_context.get_device()->submit(
        core::move(submit_infos), core::move(present_infos));
}

rhi::SwapchainHandle VulkanRHIContext::create_swapchain(
    const rhi::SwapchainCreateInfo& create_info) noexcept
{
    TNDR_PROFILER_TRACE("VulkanRHIContext::create_swapchain");

    return m_vulkan_context.get_device()->create_swapchain(create_info);
}

void VulkanRHIContext::destroy_swapchain(const rhi::SwapchainHandle handle) noexcept
{
    TNDR_PROFILER_TRACE("VulkanRHIContext::destroy_swapchain");

    m_vulkan_context.get_device()->destroy_swapchain(handle);
}

rhi::BufferHandle VulkanRHIContext::create_buffer(
    const rhi::BufferCreateInfo& create_info) noexcept
{
    TNDR_PROFILER_TRACE("VulkanRHIContext::create_buffer");

    return m_vulkan_context.get_device()->create_buffer(create_info);
}

void VulkanRHIContext::update_buffer(
    const rhi::BufferHandle handle,
    const core::Array<rhi::BufferUpdateRegion>& update_regions) noexcept
{
    TNDR_PROFILER_TRACE("VulkanRHIContext::update_buffer");

    m_vulkan_context.get_device()->update_buffer(handle, update_regions);
}

void VulkanRHIContext::destroy_buffer(const rhi::BufferHandle handle) noexcept
{
    TNDR_PROFILER_TRACE("VulkanRHIContext::destroy_buffer");

    m_vulkan_context.get_device()->destroy_buffer(handle);
}

rhi::TextureHandle VulkanRHIContext::create_texture(
    const rhi::TextureCreateInfo& create_info) noexcept
{
    TNDR_PROFILER_TRACE("VulkanRHIContext::create_texture");

    return m_vulkan_context.get_device()->create_texture(create_info);
}

void VulkanRHIContext::destroy_texture(const rhi::TextureHandle handle) noexcept
{
    TNDR_PROFILER_TRACE("VulkanRHIContext::destroy_texture");

    m_vulkan_context.get_device()->destroy_texture(handle);
}

rhi::TextureViewHandle VulkanRHIContext::create_texture_view(
    const rhi::TextureViewCreateInfo& create_info) noexcept
{
    TNDR_PROFILER_TRACE("VulkanRHIContext::create_texture_view");

    return m_vulkan_context.get_device()->create_texture_view(create_info);
}

void VulkanRHIContext::destroy_texture_view(const rhi::TextureViewHandle handle) noexcept
{
    TNDR_PROFILER_TRACE("VulkanRHIContext::destroy_texture_view");

    m_vulkan_context.get_device()->destroy_texture_view(handle);
}

rhi::ShaderHandle VulkanRHIContext::create_shader(
    const rhi::ShaderCreateInfo& create_info) noexcept
{
    TNDR_PROFILER_TRACE("VulkanRHIContext::create_shader");

    return m_vulkan_context.get_device()->create_shader(create_info);
}

void VulkanRHIContext::destroy_shader(const rhi::ShaderHandle handle) noexcept
{
    TNDR_PROFILER_TRACE("VulkanRHIContext::destroy_shader");

    m_vulkan_context.get_device()->destroy_shader(handle);
}

rhi::GraphicsPipelineHandle VulkanRHIContext::create_graphics_pipeline(
    const rhi::GraphicsPipelineCreateInfo& create_info) noexcept
{
    TNDR_PROFILER_TRACE("VulkanRHIContext::create_graphics_pipeline");

    return m_vulkan_context.get_device()->create_graphics_pipeline(create_info);
}

void VulkanRHIContext::destroy_graphics_pipeline(
    const rhi::GraphicsPipelineHandle handle) noexcept
{
    TNDR_PROFILER_TRACE("VulkanRHIContext::destroy_graphics_pipeline");

    m_vulkan_context.get_device()->destroy_graphics_pipeline(handle);
}

rhi::ComputePipelineHandle VulkanRHIContext::create_compute_pipeline(
    const rhi::ComputePipelineCreateInfo& create_info) noexcept
{
    TNDR_PROFILER_TRACE("VulkanRHIContext::create_compute_pipeline");

    return m_vulkan_context.get_device()->create_compute_pipeline(create_info);
}

void VulkanRHIContext::destroy_compute_pipeline(
    const rhi::ComputePipelineHandle handle) noexcept
{
    TNDR_PROFILER_TRACE("VulkanRHIContext::destroy_compute_pipeline");

    m_vulkan_context.get_device()->destroy_compute_pipeline(handle);
}

rhi::SamplerHandle VulkanRHIContext::create_sampler(
    const rhi::SamplerCreateInfo& create_info) noexcept
{
    TNDR_PROFILER_TRACE("VulkanRHIContext::create_sampler");

    return m_vulkan_context.get_device()->create_sampler(create_info);
}

void VulkanRHIContext::destroy_sampler(const rhi::SamplerHandle handle) noexcept
{
    TNDR_PROFILER_TRACE("VulkanRHIContext::destroy_sampler");

    m_vulkan_context.get_device()->destroy_sampler(handle);
}

} // namespace tundra::vulkan_rhi
