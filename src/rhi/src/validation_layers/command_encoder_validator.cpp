#include "validation_layers/command_encoder_validator.h"
#include "core/std/assert.h"
#include "rhi/commands/command_encoder.h"
#include "rhi/commands/dispatch_indirect.h"
#include "rhi/commands/draw_indirect.h"
#include "rhi/validation_layers.h"
#include <algorithm>

namespace tundra::rhi {

///
class CommandEncoderValidator {
private:
    ValidationLayers* m_validation_layers;
    const CommandEncoder& m_command_encoder;

private:
    struct {
        bool is_in_recording_state = false;
        bool is_in_render_pass = false;
        bool is_scissor_defined = false;
        bool is_culling_mode_defined = false;
        bool is_viewport_defined = false;
        bool is_graphics_pipeline_binded = false;
        bool is_index_buffer_binded = false;
    } m_encoder_state;

public:
    CommandEncoderValidator(
        ValidationLayers* validation_layers, const CommandEncoder& encoder) noexcept;

public:
    void validate() noexcept;

private:
    void begin_command_buffer(
        const rhi::commands::BeginCommandBufferCommand& cmd) noexcept;
    void end_command_buffer(const rhi::commands::EndCommandBufferCommand& cmd) noexcept;
    void begin_region(const rhi::commands::BeginRegionCommand& cmd) noexcept;
    void end_region(const rhi::commands::EndRegionCommand& cmd) noexcept;
    void begin_render_pass(const rhi::commands::BeginRenderPassCommand& cmd) noexcept;
    void end_render_pass(const rhi::commands::EndRenderPassCommand& cmd) noexcept;
    void push_constants(const rhi::commands::PushConstantsCommand& cmd) noexcept;
    void bind_graphics_pipeline(
        const rhi::commands::BindGraphicsPipelineCommand& cmd) noexcept;
    void set_viewport(const rhi::commands::SetViewportCommand& cmd) noexcept;
    void set_scissor(const rhi::commands::SetScissorCommand& cmd) noexcept;
    void set_culling_mode(const rhi::commands::SetCullingModeCommand& cmd) noexcept;
    void bind_index_buffer(const rhi::commands::BindIndexBufferCommand& cmd) noexcept;
    void draw(const rhi::commands::DrawCommand& cmd) noexcept;
    void draw_indexed(const rhi::commands::DrawIndexedCommand& cmd) noexcept;
    void draw_indexed_instanced(
        const rhi::commands::DrawIndexedInstancedCommand& cmd) noexcept;
    void draw_indexed_indirect(
        const rhi::commands::DrawIndexedIndirectCommand& cmd) noexcept;
    void draw_indexed_indirect_count(
        const rhi::commands::DrawIndexedIndirectCountCommand& cmd) noexcept;
    void dispatch(const rhi::commands::DispatchCommand& cmd) noexcept;
    void dispatch_indirect(const rhi::commands::DispatchIndirectCommand& cmd) noexcept;
    void buffer_copy(const rhi::commands::BufferCopyCommand& cmd) noexcept;
    void texture_copy(const rhi::commands::TextureCopyCommand& cmd) noexcept;
    void buffer_texture_copy(const rhi::commands::BufferTextureCopyCommand& cmd) noexcept;
    void texture_buffer_copy(const rhi::commands::TextureBufferCopyCommand& cmd) noexcept;
    void global_barrier(const rhi::commands::GlobalBarrierCommand& cmd) noexcept;
    void texture_barrier(const rhi::commands::TextureBarrierCommand& cmd) noexcept;
    void buffer_barrier(const rhi::commands::BufferBarrierCommand& cmd) noexcept;

private:
    [[nodiscard]] bool validate_access_flags(
        const rhi::AccessFlags access_flags,
        const rhi::BufferUsageFlags buffer_usage) noexcept;
    [[nodiscard]] bool validate_access_flags(
        const rhi::AccessFlags access_flags,
        const rhi::TextureUsageFlags texture_usage) noexcept;
};

CommandEncoderValidator::CommandEncoderValidator(
    ValidationLayers* validation_layers, const CommandEncoder& command_encoder) noexcept
    : m_validation_layers(validation_layers)
    , m_command_encoder(command_encoder)
{
}

void CommandEncoderValidator::validate() noexcept
{
    m_command_encoder.execute(core::make_overload(
        [&](const rhi::commands::BeginCommandBufferCommand& cmd) {
            this->begin_command_buffer(cmd);
        },
        [&](const rhi::commands::EndCommandBufferCommand& cmd) {
            this->end_command_buffer(cmd);
        },
        [&](const rhi::commands::BeginRegionCommand& cmd) { //
            this->begin_region(cmd);
        },
        [&](const rhi::commands::EndRegionCommand& cmd) { //
            this->end_region(cmd);
        },
        [&](const rhi::commands::BeginRenderPassCommand& cmd) {
            this->begin_render_pass(cmd);
        },
        [&](const rhi::commands::EndRenderPassCommand& cmd) {
            this->end_render_pass(cmd);
        },
        [&](const rhi::commands::PushConstantsCommand& cmd) {
            this->push_constants(cmd);
        },
        [&](const rhi::commands::BindGraphicsPipelineCommand& cmd) {
            this->bind_graphics_pipeline(cmd);
        },
        [&](const rhi::commands::SetViewportCommand& cmd) { //
            this->set_viewport(cmd);
        },
        [&](const rhi::commands::SetScissorCommand& cmd) { //
            this->set_scissor(cmd);
        },
        [&](const rhi::commands::SetCullingModeCommand& cmd) {
            this->set_culling_mode(cmd);
        },
        [&](const rhi::commands::BindIndexBufferCommand& cmd) {
            this->bind_index_buffer(cmd);
        },
        [&](const rhi::commands::DrawCommand& cmd) { //
            this->draw(cmd);
        },
        [&](const rhi::commands::DrawIndexedCommand& cmd) { //
            this->draw_indexed(cmd);
        },
        [&](const rhi::commands::DrawIndexedInstancedCommand& cmd) {
            this->draw_indexed_instanced(cmd);
        },
        [&](const rhi::commands::DrawIndexedIndirectCommand& cmd) {
            this->draw_indexed_indirect(cmd);
        },
        [&](const rhi::commands::DrawIndexedIndirectCountCommand& cmd) {
            this->draw_indexed_indirect_count(cmd);
        },
        [&](const rhi::commands::DispatchCommand& cmd) { //
            this->dispatch(cmd);
        },
        [&](const rhi::commands::DispatchIndirectCommand& cmd) { //
            this->dispatch_indirect(cmd);
        },
        [&](const rhi::commands::BufferCopyCommand& cmd) { //
            this->buffer_copy(cmd);
        },
        [&](const rhi::commands::TextureCopyCommand& cmd) { //
            this->texture_copy(cmd);
        },
        [&](const rhi::commands::BufferTextureCopyCommand& cmd) { //
            this->buffer_texture_copy(cmd);
        },
        [&](const rhi::commands::TextureBufferCopyCommand& cmd) { //
            this->texture_buffer_copy(cmd);
        },
        [&](const rhi::commands::GlobalBarrierCommand& cmd) {
            this->global_barrier(cmd);
        },
        [&](const rhi::commands::TextureBarrierCommand& cmd) {
            this->texture_barrier(cmd);
        },
        [&](const rhi::commands::BufferBarrierCommand& cmd) {
            this->buffer_barrier(cmd);
        }));
}

void CommandEncoderValidator::begin_command_buffer(
    const rhi::commands::BeginCommandBufferCommand&) noexcept
{
    tndr_assert(
        !m_encoder_state.is_in_recording_state,
        "`begin_command_buffer`/`end_command_buffer` mismatch.");
    m_encoder_state.is_in_recording_state = true;
}

void CommandEncoderValidator::end_command_buffer(
    const rhi::commands::EndCommandBufferCommand&) noexcept
{
    tndr_assert(
        m_encoder_state.is_in_recording_state,
        "`begin_command_buffer`/`end_command_buffer` mismatch.");
    m_encoder_state.is_in_recording_state = false;
}

void CommandEncoderValidator::begin_region(
    const rhi::commands::BeginRegionCommand&) noexcept
{
    tndr_assert(
        m_encoder_state.is_in_recording_state,
        "Command encoder is not in the recording state.");
}

void CommandEncoderValidator::end_region(const rhi::commands::EndRegionCommand&) noexcept
{
    tndr_assert(
        m_encoder_state.is_in_recording_state,
        "Command encoder is not in the recording state.");
}

void CommandEncoderValidator::begin_render_pass(
    const rhi::commands::BeginRenderPassCommand& cmd) noexcept
{
    tndr_assert(
        m_encoder_state.is_in_recording_state,
        "Command encoder is not in the recording state.");
    tndr_assert(
        !m_encoder_state.is_in_render_pass,
        "`begin_render_pass`/`end_render_pass` mismatch.");

    {
        const i32 width = cmd.render_area.offset.x +
                          static_cast<i32>(cmd.render_area.extent.x);
        const i32 height = cmd.render_area.offset.y +
                           static_cast<i32>(cmd.render_area.extent.y);
        tndr_assert(width > 0, "`width` must be bigger than `0`.");
        tndr_assert(height > 0, "`height` must be bigger than `0`.");
    }

    const auto textures = m_validation_layers->get_textures().read();

    // Color attachments
    for (const rhi::ColorAttachment& color_attachment :
         cmd.render_pass.color_attachments) {
        tndr_assert(
            (color_attachment.texture_access == rhi::AccessFlags::GENERAL) ||
                (color_attachment.texture_access == rhi::AccessFlags::COLOR_ATTACHMENT) ||
                (color_attachment.texture_access ==
                 rhi::AccessFlags::COLOR_ATTACHMENT_WRITE),
            "`ColorAttachment::texture_access` must be equal to "
            "`AccessFlags::GENERAL`, `AccessFlags::COLOR_ATTACHMENT_WRITE` or "
            "`AccessFlags::COLOR_ATTACHMENT`.");

        core::visit(
            core::make_overload(
                [&](const TextureHandleType& handle) {
                    const auto it = textures->find(handle);
                    tndr_assert(
                        it != textures->end(),
                        "`ColorAttachment::texture` does not exist.");
                    const rhi::TextureCreateInfo& texture_create_info = it->second;

                    tndr_assert(
                        TextureKind::get_num_mips(texture_create_info.kind) == 1,
                        "Textures used inside of a render pass should have only one mip "
                        "level! You probalby want to use a texture view.");

                    tndr_assert(
                        core::holds_alternative<TextureKind::Texture2D>(
                            texture_create_info.kind),
                        "Only 2D textures are allowed as an color attachment.");

                    tndr_assert(
                        contains(
                            texture_create_info.usage,
                            TextureUsageFlags::COLOR_ATTACHMENT),
                        "Only texture with `TextureUsageFlags::COLOR_ATTACHMENT` "
                        "bit set may be used as an color attachment.");
                },
                [&](const TextureViewHandleType&) {
                    // #TODO:
                    core::unreachable();
                }),
            color_attachment.texture);

        if (color_attachment.resolve_texture.has_value()) {
            const rhi::ResolveTexture& resolve_texture = *color_attachment.resolve_texture;

            tndr_assert(
                (resolve_texture.texture_access == rhi::AccessFlags::GENERAL) ||
                    (resolve_texture.texture_access ==
                     rhi::AccessFlags::COLOR_ATTACHMENT) ||
                    (color_attachment.texture_access ==
                     rhi::AccessFlags::COLOR_ATTACHMENT_WRITE),
                "`color_attachment.resolve_texture.texture_access` must be equal "
                "to `AccessFlags::GENERAL`, `AccessFlags::COLOR_ATTACHMENT_WRITE`"
                "or `AccessFlags::COLOR_ATTACHMENT`.");

            core::visit(
                core::make_overload(
                    [&](const TextureHandleType& handle) {
                        const auto it = textures->find(handle);
                        tndr_assert(
                            it != textures->end(),
                            "`ColorAttachment::resolve_texture` does not exist.");
                        const rhi::TextureCreateInfo& texture_create_info = it->second;

                        tndr_assert(
                            TextureKind::get_num_mips(texture_create_info.kind) == 1,
                            "Textures used inside of a render pass should have only one "
                            "mip level! You probalby want to use a texture view.");

                        tndr_assert(
                            TextureKind::get_sample_count(texture_create_info.kind) ==
                                SampleCount::Count1,
                            "Textures used as an resolve attachment should have "
                            "`sample_count` set to `SampleCount::Count1`.");

                        tndr_assert(
                            core::holds_alternative<TextureKind::Texture2D>(
                                texture_create_info.kind),
                            "Only 2D textures are allowed as an color attachment.");

                        tndr_assert(
                            contains(
                                texture_create_info.usage,
                                TextureUsageFlags::COLOR_ATTACHMENT),
                            "Only texture with `TextureUsageFlags::COLOR_ATTACHMENT` bit "
                            "set may be used as an color attachment.");
                    },
                    [&](const TextureViewHandleType&) {
                        // #TODO:
                        core::unreachable();
                    }),
                resolve_texture.resolve_texture);
        }
    }

    // Depth stencil
    if (cmd.render_pass.depth_stencil_attachment.has_value()) {
        const rhi::DepthStencilAttachment& depth_stencil_attachment =
            *cmd.render_pass.depth_stencil_attachment;

        tndr_assert(
            (depth_stencil_attachment.texture_access == rhi::AccessFlags::GENERAL) ||
                (depth_stencil_attachment.texture_access ==
                 rhi::AccessFlags::DEPTH_STENCIL_ATTACHMENT) ||
                (depth_stencil_attachment.texture_access ==
                 rhi::AccessFlags::DEPTH_STENCIL_ATTACHMENT_WRITE),
            "`color_attachment.texture_access` must be equal to "
            "`AccessFlags::GENERAL`, "
            "`AccessFlags::DEPTH_STENCIL_ATTACHMENT_WRITE` or "
            "`AccessFlags::DEPTH_STENCIL_ATTACHMENT`.");

        core::visit(
            core::make_overload(
                [&](const TextureHandleType& handle) {
                    const auto it = textures->find(handle);
                    tndr_assert(
                        it != textures->end(),
                        "`DepthStencilAttachment::texture` does not exist.");
                    const rhi::TextureCreateInfo& texture_create_info = it->second;

                    tndr_assert(
                        TextureKind::get_num_mips(texture_create_info.kind) == 1,
                        "Textures used inside of a render pass should have only one "
                        "mip level! You probalby want to use a texture view.");

                    tndr_assert(
                        core::holds_alternative<TextureKind::Texture2D>(
                            texture_create_info.kind),
                        "Only 2D textures are allowed as an depth stencil attachment.");

                    tndr_assert(
                        intersects(
                            texture_create_info.usage,
                            TextureUsageFlags::DEPTH_ATTACHMENT |
                                TextureUsageFlags::STENCIL_ATTACHMENT),
                        "Only texture with `TextureUsageFlags::DEPTH_ATTACHMENT` "
                        "or `TextureUsageFlags::STENCIL_ATTACHMENT` bit set may "
                        "be used as an depth stencil attachment.");
                },
                [&](const TextureViewHandleType&) {
                    // #TODO:
                    core::unreachable();
                }),
            depth_stencil_attachment.texture);

        if (depth_stencil_attachment.resolve_texture.has_value()) {
            const rhi::ResolveTexture& resolve_texture = *depth_stencil_attachment
                                                              .resolve_texture;
            tndr_assert(
                (resolve_texture.texture_access == rhi::AccessFlags::GENERAL) ||
                    (resolve_texture.texture_access ==
                     rhi::AccessFlags::DEPTH_STENCIL_ATTACHMENT) ||
                    (depth_stencil_attachment.texture_access ==
                     rhi::AccessFlags::DEPTH_STENCIL_ATTACHMENT_WRITE),
                "`depth_stencil_attachment.resolve_texture.texture_access` must "
                "be equal to `AccessFlags::GENERAL`, "
                "`AccessFlags::DEPTH_STENCIL_ATTACHMENT_WRITE` or "
                "`AccessFlags::DEPTH_STENCIL_ATTACHMENT`.");

            core::visit(
                core::make_overload(
                    [&](const TextureHandleType& handle) {
                        const auto it = textures->find(handle);
                        tndr_assert(
                            it != textures->end(),
                            "`DepthStencilAttachment::resolve_texture` does not exist.");
                        const rhi::TextureCreateInfo& texture_create_info = it->second;

                        tndr_assert(
                            TextureKind::get_num_mips(texture_create_info.kind) == 1,
                            "Textures used inside of a render pass should have only one "
                            "mip level.");

                        tndr_assert(
                            TextureKind::get_sample_count(texture_create_info.kind) ==
                                SampleCount::Count1,
                            "Textures used as an resolve attachment should have "
                            "`sample_count` set to `SampleCount::Count1`.");

                        tndr_assert(
                            core::holds_alternative<TextureKind::Texture2D>(
                                texture_create_info.kind),
                            "Only 2D textures are allowed as an depth stencil "
                            "attachment.");

                        tndr_assert(
                            intersects(
                                texture_create_info.usage,
                                TextureUsageFlags::DEPTH_ATTACHMENT |
                                    TextureUsageFlags::STENCIL_ATTACHMENT),
                            "Only texture with "
                            "`TextureUsageFlags::DEPTH_ATTACHMENT` "
                            "or `TextureUsageFlags::STENCIL_ATTACHMENT` bit set may be "
                            "used as an depth stencil attachment.");
                    },
                    [&](const TextureViewHandleType&) {
                        // #TODO:
                        core::unreachable();
                    }),
                depth_stencil_attachment.texture);
        }
    }

    m_encoder_state.is_in_render_pass = true;
}

void CommandEncoderValidator::end_render_pass(
    const rhi::commands::EndRenderPassCommand&) noexcept
{
    tndr_assert(
        m_encoder_state.is_in_recording_state,
        "Command encoder is not in the recording state.");
    tndr_assert(
        m_encoder_state.is_in_render_pass,
        "`begin_render_pass`/`end_render_pass` mismatch.");

    m_encoder_state.is_in_render_pass = false;
}

void CommandEncoderValidator::push_constants(
    const rhi::commands::PushConstantsCommand& cmd) noexcept
{
    tndr_assert(
        m_encoder_state.is_in_recording_state,
        "Command encoder is not in the recording state.");
    tndr_assert(cmd.ubo_buffer.is_valid(), "`ubo_buffer` must be a valid handle.");
}

void CommandEncoderValidator::bind_graphics_pipeline(
    const rhi::commands::BindGraphicsPipelineCommand& cmd) noexcept
{
    tndr_assert(
        m_encoder_state.is_in_recording_state,
        "Command encoder is not in the recording state.");
    tndr_assert(cmd.pipeline.is_valid(), "`pipeline` must be a valid handle.");

    const auto graphics_pipelines = m_validation_layers->get_graphics_pipelines().read();
    const auto it = graphics_pipelines->find(cmd.pipeline.get_handle());
    tndr_assert(
        it != graphics_pipelines->end(),
        "`BindGraphicsPipelineCommand::pipeline` does not exist.");

    m_encoder_state.is_graphics_pipeline_binded = true;
}

void CommandEncoderValidator::set_viewport(
    const rhi::commands::SetViewportCommand& cmd) noexcept
{
    tndr_assert(
        m_encoder_state.is_in_recording_state,
        "Command encoder is not in the recording state.");

    // #TODO: Validate viewport size.
    m_encoder_state.is_viewport_defined = true;
}

void CommandEncoderValidator::set_scissor(
    const rhi::commands::SetScissorCommand& cmd) noexcept
{
    tndr_assert(
        m_encoder_state.is_in_recording_state,
        "Command encoder is not in the recording state.");

    // #TODO: Validate scissor size.
    m_encoder_state.is_scissor_defined = true;
}

void CommandEncoderValidator::set_culling_mode(
    const rhi::commands::SetCullingModeCommand&) noexcept
{
    tndr_assert(
        m_encoder_state.is_in_recording_state,
        "Command encoder is not in the recording state.");

    m_encoder_state.is_culling_mode_defined = true;
}

void CommandEncoderValidator::bind_index_buffer(
    const rhi::commands::BindIndexBufferCommand& cmd) noexcept
{
    tndr_assert(
        m_encoder_state.is_in_recording_state,
        "Command encoder is not in the recording state.");
    tndr_assert(
        m_encoder_state.is_in_render_pass,
        "`bind_index_buffer` must be called inside of a render pass.");
    tndr_assert(cmd.buffer.is_valid(), "`buffer` must be a valid handle.");

    const auto buffers = m_validation_layers->get_buffers().read();
    const auto it = buffers->find(cmd.buffer.get_handle());
    tndr_assert(it != buffers->end(), "`BindIndexBufferCommand::buffer` does not exist.");
    const rhi::BufferCreateInfo& buffer_create_info = it->second;

    tndr_assert(
        contains(buffer_create_info.usage, rhi::BufferUsageFlags::INDEX_BUFFER),
        "Only buffer with `BufferUsageFlags::INDEX_BUFFER` bit set may be used as an "
        "index buffer.");
    tndr_assert(
        cmd.offset < buffer_create_info.size,
        "`BindIndexBufferCommand::offset` must be less than `BufferCreateInfo::size`.");

    m_encoder_state.is_index_buffer_binded = true;
}

void CommandEncoderValidator::draw(const rhi::commands::DrawCommand&) noexcept
{
    tndr_assert(
        m_encoder_state.is_in_recording_state,
        "Command encoder is not in the recording state.");
    tndr_assert(
        m_encoder_state.is_in_render_pass,
        "`bind_index_buffer` must be called inside of a render pass.");
    tndr_assert(
        m_encoder_state.is_scissor_defined && m_encoder_state.is_viewport_defined &&
            m_encoder_state.is_culling_mode_defined,
        "Please call `set_scissor`, `set_culling_mode` and `set_viewport` before calling "
        "any draw functions.");
    tndr_assert(
        m_encoder_state.is_graphics_pipeline_binded,
        "Please call `bind_graphics_pipeline` before calling any draw "
        "functions.");
}

void CommandEncoderValidator::draw_indexed(
    const rhi::commands::DrawIndexedCommand&) noexcept
{
    tndr_assert(
        m_encoder_state.is_in_recording_state,
        "Command encoder is not in the recording state.");
    tndr_assert(
        m_encoder_state.is_in_render_pass,
        "`bind_index_buffer` must be called inside of a render pass.");
    tndr_assert(
        m_encoder_state.is_scissor_defined && m_encoder_state.is_viewport_defined &&
            m_encoder_state.is_culling_mode_defined,
        "Please call `set_scissor`, `set_culling_mode` and `set_viewport` before calling "
        "any draw functions.");
    tndr_assert(
        m_encoder_state.is_graphics_pipeline_binded,
        "Please call `bind_graphics_pipeline` before calling any draw "
        "functions.");
    tndr_assert(
        m_encoder_state.is_index_buffer_binded,
        "Please call `bind_index_buffer` before calling any `draw_indexed` "
        "functions.");
}

void CommandEncoderValidator::draw_indexed_instanced(
    const rhi::commands::DrawIndexedInstancedCommand&) noexcept
{
    tndr_assert(
        m_encoder_state.is_in_recording_state,
        "Command encoder is not in the recording state.");
    tndr_assert(
        m_encoder_state.is_in_render_pass,
        "`bind_index_buffer` must be called inside of a render pass.");
    tndr_assert(
        m_encoder_state.is_scissor_defined && m_encoder_state.is_viewport_defined &&
            m_encoder_state.is_culling_mode_defined,
        "Please call `set_scissor`, `set_culling_mode` and `set_viewport` before calling "
        "any draw functions.");
    tndr_assert(
        m_encoder_state.is_graphics_pipeline_binded,
        "Please call `bind_graphics_pipeline` before calling any draw "
        "functions.");
    tndr_assert(
        m_encoder_state.is_index_buffer_binded,
        "Please call `bind_index_buffer` before calling any `draw_indexed` "
        "functions.");
}

void CommandEncoderValidator::draw_indexed_indirect(
    const rhi::commands::DrawIndexedIndirectCommand& cmd) noexcept
{
    tndr_assert(
        m_encoder_state.is_in_recording_state,
        "Command encoder is not in the recording state.");
    tndr_assert(
        m_encoder_state.is_in_render_pass,
        "`bind_index_buffer` must be called inside of a render pass.");
    tndr_assert(
        m_encoder_state.is_scissor_defined && m_encoder_state.is_viewport_defined &&
            m_encoder_state.is_culling_mode_defined,
        "Please call `set_scissor`, `set_culling_mode` and `set_viewport` before calling "
        "any draw functions.");
    tndr_assert(
        m_encoder_state.is_graphics_pipeline_binded,
        "Please call `bind_graphics_pipeline` before calling any draw "
        "functions.");
    tndr_assert(
        m_encoder_state.is_index_buffer_binded,
        "Please call `bind_index_buffer` before calling any `draw_indexed` "
        "functions.");

    tndr_assert(cmd.buffer.is_valid(), "`buffer` must be a valid handle.");
    tndr_assert(cmd.draw_count > 0, "`draw_count` must be bigger than `0`.");

    const auto buffers = m_validation_layers->get_buffers().read();
    const auto it = buffers->find(cmd.buffer.get_handle());
    tndr_assert(
        it != buffers->end(), "`DrawIndexedIndirectCommand::buffer` does not exist.");
    const rhi::BufferCreateInfo& buffer_create_info = it->second;

    tndr_assert(
        contains(buffer_create_info.usage, rhi::BufferUsageFlags::INDIRECT_BUFFER),
        "Only buffer with `BufferUsageFlags::INDIRECT_BUFFER` bit set may be used as an "
        "indirect buffer.");
    tndr_assert(
        cmd.offset < buffer_create_info.size,
        "`DrawIndexedIndirectCommand::offset` must be less than "
        "`BufferCreateInfo::size`.");

    // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCmdDrawIndexedIndirect.html#VUID-vkCmdDrawIndexedIndirect-drawCount-00540
    const u64 required_size =
        (static_cast<u64>(cmd.stride) * (static_cast<u64>(cmd.draw_count) - 1) +
         cmd.offset + sizeof(DrawIndexedIndirectCommand));
    tndr_assert(
        required_size <= buffer_create_info.size,
        "(cmd.stride * (cmd.draw_count - 1) + cmd.offset + "
        "sizeof(DrawIndexedIndirectCommand)) must be less than or equal to "
        "`BufferCreateInfo::size`.");
}

void CommandEncoderValidator::draw_indexed_indirect_count(
    const rhi::commands::DrawIndexedIndirectCountCommand& cmd) noexcept
{
    tndr_assert(
        m_encoder_state.is_in_recording_state,
        "Command encoder is not in the recording state.");
    tndr_assert(
        m_encoder_state.is_in_render_pass,
        "`bind_index_buffer` must be called inside of a render pass.");
    tndr_assert(
        m_encoder_state.is_scissor_defined && m_encoder_state.is_viewport_defined &&
            m_encoder_state.is_culling_mode_defined,
        "Please call `set_scissor`, `set_culling_mode` and `set_viewport` before calling "
        "any draw functions.");
    tndr_assert(
        m_encoder_state.is_graphics_pipeline_binded,
        "Please call `bind_graphics_pipeline` before calling any draw "
        "functions.");
    tndr_assert(
        m_encoder_state.is_index_buffer_binded,
        "Please call `bind_index_buffer` before calling any `draw_indexed` "
        "functions.");

    tndr_assert(cmd.buffer.is_valid(), "`buffer` must be a valid handle.");
    tndr_assert(cmd.count_buffer.is_valid(), "`count_buffer` must be a valid handle.");
    tndr_assert(
        (cmd.count_buffer_offset % 4) == 0,
        "`count_buffer_offset` must be a multiple of 4.");
    tndr_assert(cmd.max_draw_count > 0, "`max_draw_count` must be bigger than `0`.");

    const auto buffers = m_validation_layers->get_buffers().read();

    {
        const auto it = buffers->find(cmd.buffer.get_handle());
        tndr_assert(
            it != buffers->end(),
            "`DrawIndexedIndirectCountCommand::buffer` does not exist.");
        const rhi::BufferCreateInfo& buffer_create_info = it->second;

        tndr_assert(
            contains(buffer_create_info.usage, rhi::BufferUsageFlags::INDIRECT_BUFFER),
            "Only buffer with `BufferUsageFlags::INDIRECT_BUFFER` bit set may be used as "
            "an indirect buffer.");
        tndr_assert(
            cmd.offset < buffer_create_info.size,
            "`DrawIndexedIndirectCountCommand::offset` must be less than "
            "`BufferCreateInfo::size`.");

        // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCmdDrawIndexedIndirect.html#VUID-vkCmdDrawIndexedIndirect-drawCount-00540
        const u64 required_size =
            (static_cast<u64>(cmd.stride) * (static_cast<u64>(cmd.max_draw_count) - 1) +
             cmd.offset + sizeof(DrawIndexedIndirectCommand));
        tndr_assert(
            required_size <= buffer_create_info.size,
            "(cmd.stride * (cmd.max_draw_count - 1) + cmd.offset + "
            "sizeof(DrawIndexedIndirectCommand)) must be less than or equal to "
            "`BufferCreateInfo::size`.");
    }

    {
        const auto it = buffers->find(cmd.count_buffer.get_handle());
        tndr_assert(
            it != buffers->end(),
            "`DrawIndexedIndirectCountCommand::buffer` does not exist.");
        const rhi::BufferCreateInfo& buffer_create_info = it->second;

        tndr_assert(
            contains(buffer_create_info.usage, rhi::BufferUsageFlags::INDIRECT_BUFFER),
            "Only buffer with `BufferUsageFlags::INDIRECT_BUFFER` bit set may be used as "
            "an indirect buffer.");
        tndr_assert(
            cmd.count_buffer_offset < buffer_create_info.size,
            "`DrawIndexedIndirectCountCommand::offset` must be less than "
            "`BufferCreateInfo::size`.");
        tndr_assert(
            (cmd.count_buffer_offset + sizeof(u32)) <= buffer_create_info.size,
            "(cmd.count_buffer_offset + sizeof(u32)) must be less than or equal to the "
            "`BufferCreateInfo::size`.");
    }
}

void CommandEncoderValidator::dispatch(const rhi::commands::DispatchCommand& cmd) noexcept
{
    tndr_assert(
        m_encoder_state.is_in_recording_state,
        "Command encoder is not in the recording state.");
    tndr_assert(
        !m_encoder_state.is_in_render_pass,
        "`dispatch` must be called outside of a render pass.");

    tndr_assert(cmd.pipeline.is_valid(), "`pipeline` must be a valid handle.");

    const auto compute_pipelines = m_validation_layers->get_compute_pipelines().read();
    const auto it = compute_pipelines->find(cmd.pipeline.get_handle());
    tndr_assert(
        it != compute_pipelines->end(), "`DispatchCommand::pipeline` does not exist.");
}

void CommandEncoderValidator::dispatch_indirect(
    const rhi::commands::DispatchIndirectCommand& cmd) noexcept
{
    tndr_assert(
        m_encoder_state.is_in_recording_state,
        "Command encoder is not in the recording state.");
    tndr_assert(
        !m_encoder_state.is_in_render_pass,
        "`dispatch` must be called outside of a render pass.");

    tndr_assert(cmd.pipeline.is_valid(), "`pipeline` must be a valid handle.");
    tndr_assert(cmd.buffer.is_valid(), "`buffer` must be a valid handle.");
    tndr_assert((cmd.offset % 4) == 0, "`offset` must be a multiple of 4.");

    {
        const auto compute_pipelines = m_validation_layers->get_compute_pipelines().read();
        const auto it = compute_pipelines->find(cmd.pipeline.get_handle());
        tndr_assert(
            it != compute_pipelines->end(),
            "`DispatchCommand::pipeline` does not exist.");
    }

    {
        const auto buffers = m_validation_layers->get_buffers().read();
        const auto it = buffers->find(cmd.buffer.get_handle());
        tndr_assert(
            it != buffers->end(), "`DispatchIndirectCommand::buffer` does not exist.");
        const rhi::BufferCreateInfo& buffer_create_info = it->second;

        tndr_assert(
            contains(buffer_create_info.usage, rhi::BufferUsageFlags::INDIRECT_BUFFER),
            "Only buffer with `BufferUsageFlags::INDIRECT_BUFFER` bit set may be used as "
            "an indirect buffer.");
        tndr_assert(
            cmd.offset < buffer_create_info.size,
            "`DispatchIndirectCommand::offset` must be less than "
            "`BufferCreateInfo::size`.");
        tndr_assert(
            (cmd.offset + sizeof(DispatchIndirectCommand)) <= buffer_create_info.size,
            "(cmd.offset + sizeof(DispatchIndirectCommand)) must be less than or equal "
            "to the `BufferCreateInfo::size`.");
    }
}

void CommandEncoderValidator::buffer_copy(
    const rhi::commands::BufferCopyCommand& cmd) noexcept
{
    tndr_assert(
        m_encoder_state.is_in_recording_state,
        "Command encoder is not in the recording state.");
    tndr_assert(
        !m_encoder_state.is_in_render_pass,
        "`buffer_copy` must be called outside of a render pass.");
    tndr_assert(cmd.src.is_valid(), "`src` must be a valid handle.");
    tndr_assert(cmd.dst.is_valid(), "`dst` must be a valid handle.");

    if (cmd.src.get_handle() == cmd.dst.get_handle()) {
        for (const rhi::BufferCopyRegion& region : cmd.regions) {
            const u64 src_end_offset = region.src_offset + region.size;
            const u64 dst_end_offset = region.dst_offset + region.size;
            tndr_assert(
                math::max(region.src_offset, region.dst_offset) <=
                    math::min(src_end_offset, dst_end_offset),
                "Copy region overlap.");
        }
    }

    const auto buffers = m_validation_layers->get_buffers().read();
    {
        const auto it = buffers->find(cmd.src.get_handle());
        tndr_assert(it != buffers->end(), "`BufferCopyCommand::src` does not exist.");
        const rhi::BufferCreateInfo& buffer_create_info = it->second;

        tndr_assert(
            contains(buffer_create_info.usage, rhi::BufferUsageFlags::TRANSFER_SOURCE),
            "Only buffer with `BufferUsageFlags::TRANSFER_SOURCE` bit set may be used as "
            "a copy source.");

        for (const rhi::BufferCopyRegion& region : cmd.regions) {
            tndr_assert(
                region.src_offset < buffer_create_info.size,
                "`BufferCopyRegion::src_offset` must be less than "
                "`BufferCreateInfo::size`.");

            tndr_assert(
                (region.src_offset + region.size) <= buffer_create_info.size,
                "`BufferCopyRegion::src_offset` + `BufferCopyRegion::size` must be "
                "less than or equal to `BufferCreateInfo::size`.");
        }
    }

    {
        const auto it = buffers->find(cmd.dst.get_handle());
        tndr_assert(it != buffers->end(), "`BufferCopyCommand::dst` does not exist.");
        const rhi::BufferCreateInfo& buffer_create_info = it->second;

        tndr_assert(
            contains(
                buffer_create_info.usage, rhi::BufferUsageFlags::TRANSFER_DESTINATION),
            "Only buffer with `BufferUsageFlags::TRANSFER_DESTINATION` bit set may be "
            "used as "
            "a copy destination.");

        for (const rhi::BufferCopyRegion& region : cmd.regions) {
            tndr_assert(
                region.dst_offset < buffer_create_info.size,
                "`BufferCopyRegion::src_offset` must be less than "
                "`BufferCreateInfo::size`.");
            tndr_assert(
                (region.dst_offset + region.size) <= buffer_create_info.size,
                "`BufferCopyRegion::src_offset` + `BufferCopyRegion::size` must be "
                "less than or equal to `BufferCreateInfo::size`.");
        }
    }
}

void CommandEncoderValidator::texture_copy(
    const rhi::commands::TextureCopyCommand& cmd) noexcept
{
    core::unreachable();
}

void CommandEncoderValidator::buffer_texture_copy(
    const rhi::commands::BufferTextureCopyCommand& cmd) noexcept
{
    tndr_assert(
        m_encoder_state.is_in_recording_state,
        "Command encoder is not in the recording state.");
    tndr_assert(
        !m_encoder_state.is_in_render_pass,
        "`copy_buffer` must be called outside of a render pass.");
    tndr_assert(cmd.src.is_valid(), "`src` must be a valid handle.");
    tndr_assert(cmd.dst.is_valid(), "`dst` must be a valid handle.");

    core::unreachable();
}

void CommandEncoderValidator::texture_buffer_copy(
    const rhi::commands::TextureBufferCopyCommand& cmd) noexcept
{
    core::unreachable();
}

void CommandEncoderValidator::global_barrier(
    const rhi::commands::GlobalBarrierCommand&) noexcept
{
    tndr_assert(
        m_encoder_state.is_in_recording_state,
        "Command encoder is not in the recording state.");
    tndr_assert(
        !m_encoder_state.is_in_render_pass,
        "`global_barrier` must be called outside of a render pass.");
}

void CommandEncoderValidator::texture_barrier(
    const rhi::commands::TextureBarrierCommand& cmd) noexcept
{
    tndr_assert(
        m_encoder_state.is_in_recording_state,
        "Command encoder is not in the recording state.");
    tndr_assert(
        !m_encoder_state.is_in_render_pass,
        "`texture_barrier` must be called outside of a render pass.");

    auto textures = m_validation_layers->get_textures().read();
    for (const TextureBarrier& barrier : cmd.barriers) {
        tndr_assert(
            barrier.source_queue.has_value() == barrier.destination_queue.has_value(),
            "Both `source_queue` and `destination_queue` must be `Some` or `None`.");

        const auto it = textures->find(barrier.texture.get_handle());
        tndr_assert(
            it != textures->end(), "`TextureCreateInfo::texture` does not exist.");
        const rhi::TextureCreateInfo& texture_create_info = it->second;

        const u32 num_layers = TextureKind::get_num_layers(texture_create_info.kind);
        const u32 num_mips = TextureKind::get_num_mips(texture_create_info.kind);

        tndr_assert(
            barrier.subresource_range.first_layer < num_layers,
            "`TextureSubresourceRange::first_layer` must be smaller than `num_layers`.");
        if (barrier.subresource_range.layer_count != REMAINING_ARRAY_LAYERS) {
            tndr_assert(
                (barrier.subresource_range.first_layer +
                 barrier.subresource_range.layer_count) <= num_layers,
                "`TextureSubresourceRange::first_layer` + "
                "`TextureSubresourceRange::layer_count` must be less or equal to "
                "`num_layers`.");
        }

        tndr_assert(
            barrier.subresource_range.first_mip_level < num_mips,
            "`TextureSubresourceRange::first_mip_level` must be smaller than "
            "`num_mips`.");
        if (barrier.subresource_range.mip_count != REMAINING_MIP_LEVELS) {
            tndr_assert(
                (barrier.subresource_range.first_mip_level +
                 barrier.subresource_range.mip_count) <= num_layers,
                "`TextureSubresourceRange::first_mip_level` + "
                "`TextureSubresourceRange::mip_count` must be less or equal to "
                "`num_mips`.");
        }

        tndr_assert(
            CommandEncoderValidator::validate_access_flags(
                barrier.previous_access, texture_create_info.usage),
            "`TextureBarrier::previous_access` is incompatible with "
            "`TextureCreateInfo::usage`.");
        tndr_assert(
            CommandEncoderValidator::validate_access_flags(
                barrier.next_access, texture_create_info.usage),
            "`TextureBarrier::next_access` is incompatible with "
            "`TextureCreateInfo::usage`.");
    }
}

void CommandEncoderValidator::buffer_barrier(
    const rhi::commands::BufferBarrierCommand& cmd) noexcept
{
    tndr_assert(
        m_encoder_state.is_in_recording_state,
        "Command encoder is not in the recording state.");
    tndr_assert(
        !m_encoder_state.is_in_render_pass,
        "`buffer_barrier` must be called outside of a render pass.");

    const auto buffers = m_validation_layers->get_buffers().read();

    for (const BufferBarrier& barrier : cmd.barriers) {
        tndr_assert(
            barrier.source_queue.has_value() == barrier.destination_queue.has_value(),
            "Both `source_queue` and `destination_queue` must be `Some` or `None`.");

        const auto it = buffers->find(barrier.buffer.get_handle());
        tndr_assert(it != buffers->end(), "`BufferBarrier::buffer` does not exist.");
        const rhi::BufferCreateInfo& buffer_create_info = it->second;

        tndr_assert(
            barrier.subresource_range.offset < buffer_create_info.size,
            "`BufferBarrier::offset` must be less than `BufferCreateInfo::size`.");

        if (barrier.subresource_range.size != WHOLE_SIZE) {
            tndr_assert(
                (barrier.subresource_range.offset + barrier.subresource_range.size) <=
                    buffer_create_info.size,
                "`BufferBarrier::subresource_range::offset` + "
                "`BufferBarrier::subresource_range::size` must be less than or equal to "
                "`BufferCreateInfo::size`.");
        }

        tndr_assert(
            CommandEncoderValidator::validate_access_flags(
                barrier.previous_access, buffer_create_info.usage),
            "`BufferBarrier::previous_access` is incompatible with "
            "`BufferCreateInfo::buffer_usage`.");
        tndr_assert(
            CommandEncoderValidator::validate_access_flags(
                barrier.next_access, buffer_create_info.usage),
            "`BufferBarrier::next_access` is incompatible with "
            "`BufferCreateInfo::buffer_usage`.");
    }
}

bool CommandEncoderValidator::validate_access_flags(
    const rhi::AccessFlags access_flags,
    const rhi::BufferUsageFlags buffer_usage) noexcept
{
    bool is_allowed = true;

    if (contains(access_flags, AccessFlags::TRANSFER_READ)) {
        is_allowed = std::min(
            is_allowed, contains(buffer_usage, BufferUsageFlags::TRANSFER_SOURCE));
    }

    if (contains(access_flags, AccessFlags::TRANSFER_WRITE)) {
        is_allowed = std::min(
            is_allowed, contains(buffer_usage, BufferUsageFlags::TRANSFER_DESTINATION));
    }

    if (intersects(access_flags, AccessFlags::SRV_GRAPHICS | AccessFlags::SRV_COMPUTE)) {
        is_allowed = std::min(is_allowed, contains(buffer_usage, BufferUsageFlags::SRV));
    }

    if (intersects(access_flags, AccessFlags::UAV_GRAPHICS | AccessFlags::UAV_COMPUTE)) {
        is_allowed = std::min(is_allowed, contains(buffer_usage, BufferUsageFlags::UAV));
    }

    // if (contains(access_flags, AccessFlags::INDIRECT_BUFFER)) {
    //     is_allowed = std::min(
    //         is_allowed, contains(buffer_usage, BufferUsageFlags::CBV));
    // }

    if (contains(access_flags, AccessFlags::INDEX_BUFFER)) {
        is_allowed = std::min(
            is_allowed, contains(buffer_usage, BufferUsageFlags::INDEX_BUFFER));
    }

    if (contains(access_flags, AccessFlags::VERTEX_BUFFER)) {
        is_allowed = std::min(
            is_allowed, contains(buffer_usage, BufferUsageFlags::VERTEX_BUFFER));
    }

    if (contains(access_flags, AccessFlags::INDIRECT_BUFFER)) {
        is_allowed = std::min(
            is_allowed, contains(buffer_usage, BufferUsageFlags::INDIRECT_BUFFER));
    }

    return is_allowed;
}

bool CommandEncoderValidator::validate_access_flags(
    const rhi::AccessFlags access_flags,
    const rhi::TextureUsageFlags texture_usage) noexcept
{
    bool is_allowed = true;

    if (intersects(access_flags, AccessFlags::COLOR_ATTACHMENT)) {
        is_allowed = std::min(
            is_allowed, contains(texture_usage, TextureUsageFlags::COLOR_ATTACHMENT));
    }

    if (intersects(access_flags, AccessFlags::DEPTH_STENCIL_ATTACHMENT)) {
        is_allowed = std::min(
            is_allowed,
            intersects(
                texture_usage,
                TextureUsageFlags::DEPTH_ATTACHMENT |
                    TextureUsageFlags::STENCIL_ATTACHMENT));
    }

    if (intersects(access_flags, AccessFlags::SRV_GRAPHICS | AccessFlags::SRV_COMPUTE)) {
        is_allowed = std::min(
            is_allowed, contains(texture_usage, TextureUsageFlags::SRV));
    }

    if (intersects(access_flags, AccessFlags::UAV_GRAPHICS | AccessFlags::UAV_COMPUTE)) {
        is_allowed = std::min(
            is_allowed, contains(texture_usage, TextureUsageFlags::UAV));
    }

    if (contains(access_flags, AccessFlags::TRANSFER_READ)) {
        is_allowed = std::min(
            is_allowed,
            intersects(
                texture_usage,
                TextureUsageFlags::TRANSFER_SOURCE | TextureUsageFlags::PRESENT));
    }

    if (contains(access_flags, AccessFlags::TRANSFER_WRITE)) {
        is_allowed = std::min(
            is_allowed, contains(texture_usage, TextureUsageFlags::TRANSFER_DESTINATION));
    }

    if (contains(access_flags, AccessFlags::PRESENT)) {
        is_allowed = std::min(
            is_allowed, contains(texture_usage, TextureUsageFlags::PRESENT));
    }

    return is_allowed;
}

//////////////////////////////////////////////////////////////////////////
//

void validate_command_encoder(
    ValidationLayers* validation_layers, const CommandEncoder& encoder) noexcept
{
    CommandEncoderValidator validator(validation_layers, encoder);
    validator.validate();
}

} // namespace tundra::rhi
