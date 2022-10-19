#include "rhi/validation_layers.h"
#include "core/std/assert.h"
#include "core/std/utils.h"
#include "validation_layers/command_encoder_validator.h"

namespace tundra::rhi {

ValidationLayers::ValidationLayers(core::UniquePtr<IRHIContext>&& context) noexcept
    : m_context(core::move(context))
{
}

const core::UniquePtr<IRHIContext>& ValidationLayers::get_context() const noexcept
{
    return m_context;
}

void ValidationLayers::submit(
    core::Array<SubmitInfo> submit_infos, core::Array<PresentInfo> present_infos) noexcept
{
    for (const SubmitInfo& submit_info : submit_infos) {
        for (const CommandEncoder& encoder : submit_info.encoders) {
            validate_command_encoder(this, encoder);
        }
    }

    // #TODO: validate present infos

    // tndr_assert(
    //     core::holds_alternative<rhi::TextureKind::Texture2D>(t.get_texture_kind()),
    //     "Only `TextureKind::Texture2D` is allowed!");

    return m_context->submit(core::move(submit_infos), core::move(present_infos));
}

const char* ValidationLayers::get_name() const noexcept
{
    return m_context->get_name();
}

GraphicsAPI ValidationLayers::get_graphics_api() const noexcept
{
    return m_context->get_graphics_api();
}

QueueFamilyIndices ValidationLayers::get_queue_family_indices() const noexcept
{
    return m_context->get_queue_family_indices();
}

SwapchainHandle ValidationLayers::create_swapchain(
    const SwapchainCreateInfo& create_info) noexcept
{
    return m_context->create_swapchain(create_info);
}

void ValidationLayers::destroy_swapchain(const SwapchainHandle handle) noexcept
{
    m_context->destroy_swapchain(handle);
}

BufferHandle ValidationLayers::create_buffer(const BufferCreateInfo& create_info) noexcept
{
    tndr_assert(create_info.size > 0, "`create_info.size` must be greater than `0`!");

    const BufferHandle handle = m_context->create_buffer(create_info);

    auto buffers = m_buffers.write();
    tndr_assert(!buffers->contains(handle.get_handle()), "");
    buffers->insert({ handle.get_handle(), create_info });

    return handle;
}

void ValidationLayers::update_buffer(
    const BufferHandle handle,
    const core::Array<BufferUpdateRegion>& update_regions) noexcept
{
    m_context->update_buffer(handle, update_regions);
}

void ValidationLayers::destroy_buffer(const BufferHandle handle) noexcept
{
    tndr_assert(handle.is_valid(), "`handle` must be a valid handle!");

    auto buffers = m_buffers.write();
    const auto it = buffers->find(handle.get_handle());
    tndr_assert(it != buffers->end(), "");
    buffers->erase(it);

    m_context->destroy_buffer(handle);
}

TextureHandle ValidationLayers::create_texture(
    const TextureCreateInfo& create_info) noexcept
{
    tndr_assert(
        !(intersects(
              get_texture_format_desc(create_info.format).aspect,
              TextureAspectFlags::DEPTH | TextureAspectFlags::STENCIL) &&
          intersects(create_info.usage, TextureUsageFlags::COLOR_ATTACHMENT)),
        "Depth/stencil formats cannot be used with `TextureUsage::COLOR_ATTACHMENT`!");

    tndr_assert(
        !(intersects(
              get_texture_format_desc(create_info.format).aspect,
              TextureAspectFlags::COLOR) &&
          intersects(
              create_info.usage,
              TextureUsageFlags::DEPTH_ATTACHMENT |
                  TextureUsageFlags::STENCIL_ATTACHMENT)),
        "Color formats cannot be used with `TextureUsage::DEPTH_ATTACHMENT | "
        "TextureUsage::STENCIL_ATTACHMENT`!");

    [[maybe_unused]] const Extent texture_extent = TextureKind::get_extent(
        create_info.kind);
    tndr_assert(texture_extent.width > 0, "Texture width must be bigger than 0.");
    tndr_assert(texture_extent.height > 0, "Texture height must be bigger than 0.");

    const auto handle = m_context->create_texture(create_info);

    auto textures = m_textures.write();
    tndr_assert(!textures->contains(handle.get_handle()), "");
    textures->insert({ handle.get_handle(), create_info });

    return handle;
}

void ValidationLayers::destroy_texture(const TextureHandle handle) noexcept
{
    tndr_assert(handle.is_valid(), "`handle` must be a valid handle!");

    auto textures = m_textures.write();
    const auto it = textures->find(handle.get_handle());
    tndr_assert(it != textures->end(), "");
    textures->erase(it);

    m_context->destroy_texture(handle);
}

rhi::TextureViewHandle ValidationLayers::create_texture_view(
    const rhi::TextureViewCreateInfo& create_info) noexcept
{
    // #TODO: Add validation

    return m_context->create_texture_view(create_info);
}

void ValidationLayers::destroy_texture_view(const rhi::TextureViewHandle handle) noexcept
{
    tndr_assert(handle.is_valid(), "`handle` must be a valid handle!");

    m_context->destroy_texture_view(handle);
}

ShaderHandle ValidationLayers::create_shader(const ShaderCreateInfo& create_info) noexcept
{
    tndr_assert(
        create_info.shader_stage != ShaderStage::Invalid,
        "`create_info.shader_stage` must not be equal to `ShaderStage::Invalid`!");
    tndr_assert(
        !create_info.shader_buffer.is_empty(),
        "`create_info.shader_buffer` must not be empty!");
    tndr_assert(
        (create_info.shader_buffer.size() % 4) == 0,
        "`create_info.shader_buffer` must be aligned to 4 bytes!");

    return m_context->create_shader(create_info);
}

void ValidationLayers::destroy_shader(const ShaderHandle handle) noexcept
{
    tndr_assert(handle.is_valid(), "`handle` must be a valid handle!");

    m_context->destroy_shader(handle);
}

GraphicsPipelineHandle ValidationLayers::create_graphics_pipeline(
    const GraphicsPipelineCreateInfo& create_info) noexcept
{
    const rhi::GraphicsPipelineHandle handle = m_context->create_graphics_pipeline(
        create_info);

    auto graphics_pipelines = m_graphics_pipelines.write();
    tndr_assert(!graphics_pipelines->contains(handle.get_handle()), "");
    graphics_pipelines->insert({ handle.get_handle(), create_info });

    return handle;
}

void ValidationLayers::destroy_graphics_pipeline(
    const GraphicsPipelineHandle handle) noexcept
{
    tndr_assert(handle.is_valid(), "`handle` must be a valid handle!");

    auto graphics_pipelines = m_graphics_pipelines.write();
    const auto it = graphics_pipelines->find(handle.get_handle());
    tndr_assert(it != graphics_pipelines->end(), "");
    graphics_pipelines->erase(it);

    m_context->destroy_graphics_pipeline(handle);
}

ComputePipelineHandle ValidationLayers::create_compute_pipeline(
    const ComputePipelineCreateInfo& create_info) noexcept
{
    const rhi::ComputePipelineHandle handle = m_context->create_compute_pipeline(
        create_info);

    auto compute_pipelines = m_compute_pipelines.write();
    tndr_assert(!compute_pipelines->contains(handle.get_handle()), "");
    compute_pipelines->insert({ handle.get_handle(), create_info });

    return handle;
}

void ValidationLayers::destroy_compute_pipeline(
    const ComputePipelineHandle handle) noexcept
{
    tndr_assert(handle.is_valid(), "`handle` must be a valid handle!");

    auto compute_pipelines = m_compute_pipelines.write();
    const auto it = compute_pipelines->find(handle.get_handle());
    tndr_assert(it != compute_pipelines->end(), "");
    compute_pipelines->erase(it);

    m_context->destroy_compute_pipeline(handle);
}

SamplerHandle ValidationLayers::create_sampler(
    const SamplerCreateInfo& create_info) noexcept
{
    return m_context->create_sampler(create_info);
}

void ValidationLayers::destroy_sampler(const SamplerHandle handle) noexcept
{
    tndr_assert(handle.is_valid(), "`handle` must be a valid handle!");

    m_context->destroy_sampler(handle);
}

} // namespace tundra::rhi
