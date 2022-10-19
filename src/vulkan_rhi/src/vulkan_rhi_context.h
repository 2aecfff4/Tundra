#pragma once
#include "rhi/rhi_context.h"
#include "vulkan_context.h"

namespace tundra::vulkan_rhi {

class VulkanRHIContext final : public rhi::IRHIContext {
private:
    VulkanContext m_vulkan_context;

public:
    virtual const char* get_name() const noexcept final;
    [[nodiscard]] virtual rhi::GraphicsAPI get_graphics_api() const noexcept final;
    [[nodiscard]] virtual rhi::QueueFamilyIndices get_queue_family_indices()
        const noexcept final;

public:
    virtual void submit(
        core::Array<rhi::SubmitInfo> submit_infos,
        core::Array<rhi::PresentInfo> present_infos) noexcept final;

public:
    [[nodiscard]] virtual rhi::SwapchainHandle create_swapchain(
        const rhi::SwapchainCreateInfo& create_info) noexcept final;
    virtual void destroy_swapchain(const rhi::SwapchainHandle handle) noexcept final;

    [[nodiscard]] virtual rhi::BufferHandle create_buffer(
        const rhi::BufferCreateInfo& create_info) noexcept final;
    virtual void update_buffer(
        const rhi::BufferHandle handle,
        const core::Array<rhi::BufferUpdateRegion>& update_regions) noexcept final;
    virtual void destroy_buffer(const rhi::BufferHandle handle) noexcept final;

    [[nodiscard]] virtual rhi::TextureHandle create_texture(
        const rhi::TextureCreateInfo& create_info) noexcept final;
    virtual void destroy_texture(const rhi::TextureHandle handle) noexcept final;

    [[nodiscard]] virtual rhi::TextureViewHandle create_texture_view(
        const rhi::TextureViewCreateInfo& create_info) noexcept final;
    virtual void destroy_texture_view(const rhi::TextureViewHandle handle) noexcept final;

    [[nodiscard]] virtual rhi::ShaderHandle create_shader(
        const rhi::ShaderCreateInfo& create_info) noexcept final;
    virtual void destroy_shader(const rhi::ShaderHandle handle) noexcept final;

    [[nodiscard]] virtual rhi::GraphicsPipelineHandle create_graphics_pipeline(
        const rhi::GraphicsPipelineCreateInfo& create_info) noexcept final;
    virtual void destroy_graphics_pipeline(
        const rhi::GraphicsPipelineHandle handle) noexcept final;

    [[nodiscard]] virtual rhi::ComputePipelineHandle create_compute_pipeline(
        const rhi::ComputePipelineCreateInfo& create_info) noexcept final;
    virtual void destroy_compute_pipeline(
        const rhi::ComputePipelineHandle handle) noexcept final;

    [[nodiscard]] virtual rhi::SamplerHandle create_sampler(
        const rhi::SamplerCreateInfo& create_info) noexcept final;
    virtual void destroy_sampler(const rhi::SamplerHandle handle) noexcept final;
};

} // namespace tundra::vulkan_rhi
