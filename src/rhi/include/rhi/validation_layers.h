#pragma once
#include "rhi/rhi_export.h"
#include "core/std/containers/hash_map.h"
#include "core/std/sync/rw_lock.h"
#include "core/std/unique_ptr.h"
#include "rhi/rhi_context.h"

namespace tundra::rhi {

///
class RHI_API ValidationLayers : public IRHIContext {
private:
    core::UniquePtr<IRHIContext> m_context;

public:
    ValidationLayers(core::UniquePtr<IRHIContext>&& context) noexcept;

public:
    [[nodiscard]] const core::UniquePtr<IRHIContext>& get_context() const noexcept;

public: // IRHIContext
    virtual void submit(
        core::Array<SubmitInfo> submit_infos,
        core::Array<PresentInfo> present_infos) noexcept final;
    virtual const char* get_name() const noexcept final;
    [[nodiscard]] virtual GraphicsAPI get_graphics_api() const noexcept final;
    [[nodiscard]] virtual QueueFamilyIndices get_queue_family_indices()
        const noexcept final;
    [[nodiscard]] virtual SwapchainHandle create_swapchain(
        const SwapchainCreateInfo& create_info) noexcept final;
    virtual void destroy_swapchain(const SwapchainHandle handle) noexcept final;
    [[nodiscard]] virtual BufferHandle create_buffer(
        const BufferCreateInfo& create_info) noexcept final;
    virtual void update_buffer(
        const BufferHandle handle,
        const core::Array<BufferUpdateRegion>& update_regions) noexcept final;
    virtual void destroy_buffer(const BufferHandle handle) noexcept final;
    [[nodiscard]] virtual TextureHandle create_texture(
        const TextureCreateInfo& create_info) noexcept final;
    virtual void destroy_texture(const TextureHandle handle) noexcept final;
    [[nodiscard]] virtual rhi::TextureViewHandle create_texture_view(
        const rhi::TextureViewCreateInfo& create_info) noexcept final;
    virtual void destroy_texture_view(const rhi::TextureViewHandle handle) noexcept final;
    [[nodiscard]] virtual ShaderHandle create_shader(
        const ShaderCreateInfo& create_info) noexcept final;
    virtual void destroy_shader(const ShaderHandle handle) noexcept final;
    [[nodiscard]] virtual GraphicsPipelineHandle create_graphics_pipeline(
        const GraphicsPipelineCreateInfo& create_info) noexcept final;
    virtual void destroy_graphics_pipeline(
        const GraphicsPipelineHandle handle) noexcept final;
    [[nodiscard]] virtual ComputePipelineHandle create_compute_pipeline(
        const ComputePipelineCreateInfo& create_info) noexcept final;
    virtual void destroy_compute_pipeline(
        const ComputePipelineHandle handle) noexcept final;
    [[nodiscard]] virtual SamplerHandle create_sampler(
        const SamplerCreateInfo& create_info) noexcept final;
    virtual void destroy_sampler(const SamplerHandle handle) noexcept final;

private:
    [[nodiscard]] auto& get_textures() noexcept
    {
        return m_textures;
    }

    [[nodiscard]] auto& get_buffers() noexcept
    {
        return m_buffers;
    }

    [[nodiscard]] auto& get_compute_pipelines() noexcept
    {
        return m_compute_pipelines;
    }

    [[nodiscard]] auto& get_graphics_pipelines() noexcept
    {
        return m_graphics_pipelines;
    }

private:
    core::RwLock<core::HashMap<TextureHandleType, TextureCreateInfo>> m_textures;
    core::RwLock<core::HashMap<BufferHandleType, BufferCreateInfo>> m_buffers;
    core::RwLock<core::HashMap<ComputePipelineHandleType, ComputePipelineCreateInfo>>
        m_compute_pipelines;
    core::RwLock<core::HashMap<GraphicsPipelineHandleType, GraphicsPipelineCreateInfo>>
        m_graphics_pipelines;

private:
    friend class CommandEncoderValidator;
};

} // namespace tundra::rhi
