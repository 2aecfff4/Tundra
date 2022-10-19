#pragma once
#include "rhi/rhi_export.h"
#include "core/core.h"
#include "core/memory/linear_allocator.h"
#include "core/std/containers/array.h"
#include "core/std/panic.h"
#include "core/std/span.h"
#include "math/vector4.h"
#include "rhi/commands/barrier.h"
#include "rhi/commands/commands.h"
#include "rhi/commands/push_constants.h"
#include "rhi/resources/buffer.h"
#include "rhi/resources/graphics_pipeline.h"
#include "rhi/resources/handle.h"
#include "rhi/resources/index_buffer.h"
#include "rhi/resources/render_pass.h"

namespace tundra::rhi {

///
class RHI_API CommandEncoder {
private:
    core::memory::LinearAllocator m_command_allocator;
    commands::BaseCommand* m_root_command;
    commands::BaseCommand** m_command_link;
    usize m_num_commands = 0;

    struct {
        bool is_in_render_pass = false;
        bool is_in_recording_state = false;
        bool is_scissor_defined = false;
        bool is_viewport_defined = false;
        bool is_graphics_pipeline_binded = false;
        bool is_index_buffer_binded = false;
    } m_state;

public:
    CommandEncoder() noexcept;
    ~CommandEncoder() noexcept;
    CommandEncoder(CommandEncoder&& rhs) noexcept;
    CommandEncoder& operator=(CommandEncoder&& rhs) noexcept;
    CommandEncoder(const CommandEncoder& rhs) noexcept = delete;
    CommandEncoder& operator=(const CommandEncoder& rhs) noexcept = delete;

public:
    /// Start recording a command encoder.
    void begin_command_buffer() noexcept;

    /// Stop recording a command encoder.
    void end_command_buffer() noexcept;

    ///
    void begin_region(core::String name, const math::Vec4& color) noexcept;

    ///
    void end_region() noexcept;

    /// Begin a new render pass
    ///
    /// @param render_area The render area that is affected by the render pass.
    /// @param render_pass The render pass to begin an instance of.
    void begin_render_pass(const Rect& render_area, RenderPass render_pass) noexcept;

    /// End the current render pass.
    void end_render_pass() noexcept;

    /// Update the values of push constants.
    void push_constants(const rhi::BufferHandle ubo_buffer, const u32 offset) noexcept;

    /// Bind a graphics pipeline to a command encoder.
    void bind_graphics_pipeline(const GraphicsPipelineHandle pipeline) noexcept;

    /// Set the viewport on a command encoder.
    void set_viewport(const Viewport& viewport) noexcept;

    /// Set the scissor on a command encoder.
    void set_scissor(const Scissor& scissor) noexcept;

    /// Set culling mode dynamically.
    void set_culling_mode(const CullingMode culling_mode = CullingMode::Back) noexcept;

    /// Bind an index buffer to a command encoder.
    void bind_index_buffer(
        const BufferHandle buffer, const u64 offset, const IndexType index_type) noexcept;

    /// Draw primitives.
    ///
    /// @param vertex_count The number of vertices to draw.
    /// @param first_vertex The index of the first vertex to draw.
    void draw(const u32 vertex_count, const u32 first_vertex) noexcept;

    /// Draw primitives with indexed vertices.
    ///
    /// @param indices_count The number of vertices to draw.
    /// @param first_index The base index within the index buffer.
    /// @param vertex_offset The value added to the vertex index before indexing into the vertex buffer. Default set to `0`.
    void draw_indexed(
        const u32 indices_count, const u32 first_index, const i32 vertex_offset) noexcept;

    /// Draw primitives with indexed vertices.
    ///
    /// @param indices_count The number of vertices to draw.
    /// @param num_instances The number of instances to draw.
    /// @param first_index The base index within the index buffer.
    /// @param vertex_offset The value added to the vertex index before indexing into the vertex buffer.
    /// @param first_instance The instance ID of the first instance to draw.
    void draw_indexed_instanced(
        const u32 indices_count,
        const u32 num_instances,
        const u32 first_index,
        const i32 vertex_offset,
        const u32 first_instance) noexcept;

    /// Draw primitives with indirect parameters and indexed vertices.
    ///
    /// @param buffer The buffer containing draw parameters.
    /// @param offset The byte offset into `buffer` where parameters begin.
    /// @param draw_count The number of draws to execute. Can be zero.
    /// @param stride The byte stride between successive sets of draw parameters.
    void draw_indexed_indirect(
        const BufferHandle buffer,
        const u64 offset,
        const u32 draw_count,
        const u32 stride) noexcept;

    /// Draw primitives with indirect parameters, indexed vertices, and draw count.
    ///
    /// @param buffer The buffer containing draw parameters.
    /// @param offset The byte offset into `buffer` where parameters begin.
    /// @param count_buffer The buffer containing the draw count.
    /// @param count_buffer_offset The byte offset into `count_buffer` where the draw count begins.
    /// @param max_draw_count The maximum number of draws that will be executed.
    ///     The actual number of executed draw calls is the minimum of the count specified in `count_buffer` and `max_draw_count`.
    /// @param stride The byte stride between successive sets of draw parameters.
    void draw_indexed_indirect_count(
        const BufferHandle buffer,
        const u64 offset,
        const BufferHandle count_buffer,
        const u64 count_buffer_offset,
        const u32 max_draw_count,
        const u32 stride) noexcept;

    /// Dispatch compute work items.
    ///
    /// @param pipeline Valid compute pipeline handle.
    /// @param group_count_x The number of local workgroups to dispatch in the `X` dimension.
    /// @param group_count_y The number of local workgroups to dispatch in the `Y` dimension.
    /// @param group_count_z The number of local workgroups to dispatch in the `Z` dimension.
    ///
    /// @note For calculating group counts please use [`CommandEncoder::get_group_count`].
    void dispatch(
        const ComputePipelineHandle pipeline,
        const u32 group_count_x,
        const u32 group_count_y,
        const u32 group_count_z) noexcept;

    /// Dispatch compute work items with indirect parameters
    ///
    /// @param pipeline Valid compute pipeline handle.
    /// @param buffer The buffer containing dispatch parameters.
    /// @param offset The byte offset into `buffer` where parameters begin.
    ///
    /// @note For calculating group counts please use [`CommandEncoder::get_group_count`].
    void dispatch_indirect(
        const ComputePipelineHandle pipeline,
        const BufferHandle buffer,
        const u64 offset) noexcept;

    /// Copies data between buffer region.
    /// The source and the destination buffer can be the same, but copy regions must not overlap.
    /// @param src The source buffer.
    /// @param dst The destination buffer.
    /// @param regions The regions to copy
    void buffer_copy(
        const BufferHandle src,
        const BufferHandle dst,
        core::Array<BufferCopyRegion> regions) noexcept;

    /// Copies regions from the source to the destination textures.
    /// The source and the destination texture can be the same, but copy regions must not overlap.
    /// No format conversion is done.
    /// The source and destination texture format must have the same `TextureFormatDesc::num_bits`.
    ///
    /// @param src_texture_access Texture access. Used to determine the current texture layout.
    ///     Must be `AccessFlags::TRANSFER_READ` or `AccessFlags::GENERAL`.
    /// @param dst_texture_access Texture access. Used to determine the current texture layout.
    ///     Must be `AccessFlags::TRANSFER_WRITE` or `AccessFlags::GENERAL`.
    void texture_copy(
        const TextureHandle src,
        const AccessFlags src_texture_access,
        const TextureHandle dst,
        const AccessFlags dst_texture_access,
        core::Array<TextureCopyRegion> regions) noexcept;

    /// Copies regions from the source buffer to the destination texture.
    void copy_buffer_to_texture(
        const BufferHandle src,
        const TextureHandle dst,
        core::Array<BufferTextureCopyRegion> regions) noexcept;

    /// Copies regions from the source image to the destination buffer.
    void copy_texture_to_buffer(
        const TextureHandle src,
        const BufferHandle dst,
        core::Array<BufferTextureCopyRegion> regions) noexcept;

    ///
    void global_barrier(const GlobalBarrier& barrier) noexcept;

    ///
    void texture_barrier(core::Array<TextureBarrier> barriers) noexcept;

    ///
    void buffer_barrier(core::Array<BufferBarrier> barriers) noexcept;

public:
    /// Reset a command encoder to the initial state.
    void reset() noexcept;

public:
    [[nodiscard]] bool has_commands() const noexcept;

public:
    [[nodiscard]] static constexpr u32 get_group_count(
        const u32 count, const u32 local_size) noexcept
    {
        return (count + local_size - 1) / local_size;
    }

private:
    template <typename Cmd>
    void construct_command(Cmd&& cmd) noexcept;

public:
    template <typename Func>
    void execute(Func&& func) const noexcept
    {
        if (this->has_commands()) {
            const commands::BaseCommand* TNDR_RESTRICT command = m_root_command;
            [[likely]] while (command != nullptr)
            {
                switch (command->get_type()) {
#define CASE(e)                                                                          \
    case commands::CommandType::e: {                                                     \
        const auto& cmd = static_cast<const commands::TNDR_APPEND(e, Command)&>(         \
            *command);                                                                   \
        func(cmd);                                                                       \
        break;                                                                           \
    }
                    CASE(BeginCommandBuffer)
                    CASE(EndCommandBuffer)
                    CASE(BeginRegion)
                    CASE(EndRegion)
                    CASE(BeginRenderPass)
                    CASE(EndRenderPass)
                    CASE(PushConstants)
                    CASE(BindGraphicsPipeline)
                    CASE(SetViewport)
                    CASE(SetScissor)
                    CASE(SetCullingMode)
                    CASE(BindIndexBuffer)
                    CASE(Draw)
                    CASE(DrawIndexed)
                    CASE(DrawIndexedInstanced)
                    CASE(DrawIndexedIndirect)
                    CASE(DrawIndexedIndirectCount)
                    CASE(Dispatch)
                    CASE(DispatchIndirect)
                    CASE(BufferCopy)
                    CASE(TextureCopy)
                    CASE(BufferTextureCopy)
                    CASE(TextureBufferCopy)
                    CASE(GlobalBarrier)
                    CASE(TextureBarrier)
                    CASE(BufferBarrier)
#undef CASE
                    default:
                        core::panic("Invalid command type!");
                }
                command = command->next;
            }
        }
    }
};

} // namespace tundra::rhi
