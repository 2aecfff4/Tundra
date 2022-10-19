#pragma once
#include "rhi/rhi_export.h"
#include "core/std/containers/array.h"
#include "rhi/queue.h"
#include "rhi/resources/buffer.h"
#include "rhi/resources/compute_pipeline.h"
#include "rhi/resources/graphics_pipeline.h"
#include "rhi/resources/handle.h"
#include "rhi/resources/sampler.h"
#include "rhi/resources/shader.h"
#include "rhi/resources/swapchain.h"
#include "rhi/resources/texture.h"
#include "rhi/submit_info.h"

namespace tundra::rhi {

///
class RHI_API IRHIContext {
public:
    IRHIContext() noexcept = default;
    virtual ~IRHIContext() noexcept = default;
    IRHIContext(IRHIContext&&) noexcept = delete;
    IRHIContext& operator=(IRHIContext&&) noexcept = delete;
    IRHIContext(const IRHIContext&) noexcept = delete;
    IRHIContext& operator=(const IRHIContext&) noexcept = delete;

public:
    [[nodiscard]] virtual const char* get_name() const noexcept = 0;
    [[nodiscard]] virtual GraphicsAPI get_graphics_api() const noexcept = 0;
    [[nodiscard]] virtual QueueFamilyIndices get_queue_family_indices() const noexcept = 0;

public:
    ///
    virtual void submit(
        core::Array<SubmitInfo> submit_infos,
        core::Array<PresentInfo> present_infos) noexcept = 0;

    /// Returns a valid handle to a swapchain.
    [[nodiscard]] virtual SwapchainHandle create_swapchain(
        const SwapchainCreateInfo& create_info) noexcept = 0;

    /// Destroy a swapchain.
    ///
    /// @param handle A valid handle to a swapchain.
    virtual void destroy_swapchain(const SwapchainHandle handle) noexcept = 0;

    /// Returns a valid handle to a buffer.
    [[nodiscard]] virtual BufferHandle create_buffer(
        const BufferCreateInfo& create_info) noexcept = 0;

    ///
    virtual void update_buffer(
        const BufferHandle handle,
        const core::Array<BufferUpdateRegion>& update_regions) noexcept = 0;

    /// Destroy a buffer.
    ///
    /// @param handle A valid handle to a buffer.
    virtual void destroy_buffer(const BufferHandle handle) noexcept = 0;

    /// Returns a valid handle to a texture.
    [[nodiscard]] virtual TextureHandle create_texture(
        const TextureCreateInfo& create_info) noexcept = 0;

    /// Destroy a texture.
    /// # Arguments
    /// * `handle` - A valid handle to a texture.
    virtual void destroy_texture(const TextureHandle handle) noexcept = 0;

    /// Returns a valid handle to a texture view.
    [[nodiscard]] virtual TextureViewHandle create_texture_view(
        const TextureViewCreateInfo& create_info) noexcept = 0;

    /// Destroy a texture.
    /// # Arguments
    /// * `handle` - A valid handle to a texture view.
    virtual void destroy_texture_view(const TextureViewHandle handle) noexcept = 0;

    /// Returns a valid handle to a shader.
    [[nodiscard]] virtual ShaderHandle create_shader(
        const ShaderCreateInfo& create_info) noexcept = 0;

    /// Destroy a shader.
    ///
    /// @param handle A valid handle to a shader.
    virtual void destroy_shader(const ShaderHandle handle) noexcept = 0;

    /// Returns a valid handle to a graphics pipeline.
    [[nodiscard]] virtual GraphicsPipelineHandle create_graphics_pipeline(
        const GraphicsPipelineCreateInfo& create_info) noexcept = 0;

    /// Destroy a graphics pipeline.
    ///
    /// @param handle A valid handle to a graphics pipeline.
    virtual void destroy_graphics_pipeline(
        const GraphicsPipelineHandle handle) noexcept = 0;

    /// Returns a valid handle to a compute pipeline.
    [[nodiscard]] virtual ComputePipelineHandle create_compute_pipeline(
        const ComputePipelineCreateInfo& create_info) noexcept = 0;

    /// Destroy a compute pipeline.
    ///
    /// @param handle A valid handle to a compute pipeline.
    virtual void destroy_compute_pipeline(const ComputePipelineHandle handle) noexcept = 0;

    /// Returns a valid handle to a sampler.
    [[nodiscard]] virtual SamplerHandle create_sampler(
        const SamplerCreateInfo& create_info) noexcept = 0;

    /// Destroy a sampler.
    ///
    /// @param handle A valid handle to a sampler.
    virtual void destroy_sampler(const SamplerHandle handle) noexcept = 0;
};

} // namespace tundra::rhi
