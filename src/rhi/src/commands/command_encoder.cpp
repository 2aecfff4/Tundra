#include "rhi/commands/command_encoder.h"
#include "core/std/panic.h"
#include "core/std/utils.h"
#include "rhi/commands/commands.h"
#include "rhi/config.h"
#include <cstring>

namespace tundra::rhi {

CommandEncoder::CommandEncoder() noexcept
    : m_command_allocator(config::COMMAND_BUFFER_SIZE)
    , m_root_command(
          static_cast<commands::BaseCommand*>(m_command_allocator.get_buffer()))
    , m_command_link(&m_root_command)
{
}

CommandEncoder::~CommandEncoder() noexcept
{
    this->reset();
}

CommandEncoder::CommandEncoder(CommandEncoder&& rhs) noexcept
    : m_command_allocator(core::move(rhs.m_command_allocator))
    , m_root_command(core::exchange(rhs.m_root_command, nullptr))
    , m_command_link(core::exchange(rhs.m_command_link, nullptr))
    , m_num_commands(core::exchange(rhs.m_num_commands, 0))
    , m_state(core::move(rhs.m_state))
{
}

CommandEncoder& CommandEncoder::operator=(CommandEncoder&& rhs) noexcept
{
    if (&rhs != this) {
        this->reset();

        m_command_allocator = core::move(rhs.m_command_allocator);
        m_root_command = core::exchange(rhs.m_root_command, nullptr);
        m_command_link = core::exchange(rhs.m_command_link, nullptr);
        m_num_commands = core::exchange(rhs.m_num_commands, 0);
        m_state = core::move(rhs.m_state);
    }

    return *this;
}

void CommandEncoder::begin_command_buffer() noexcept
{
    this->construct_command(commands::BeginCommandBufferCommand {});
}

void CommandEncoder::end_command_buffer() noexcept
{
    this->construct_command(commands::EndCommandBufferCommand {});
}

void CommandEncoder::begin_region(core::String name, const math::Vec4& color) noexcept
{
    this->construct_command(commands::BeginRegionCommand {
        .name = core::move(name),
        .color = color,
    });
}

void CommandEncoder::end_region() noexcept
{
    this->construct_command(commands::EndRegionCommand {});
}

void CommandEncoder::begin_render_pass(
    const Rect& render_area, RenderPass render_pass) noexcept
{
    this->construct_command(commands::BeginRenderPassCommand {
        .render_area = render_area,
        .render_pass = core::move(render_pass),
    });
}

void CommandEncoder::end_render_pass() noexcept
{
    this->construct_command(commands::EndRenderPassCommand {});
}

void CommandEncoder::push_constants(
    const rhi::BufferHandle ubo_buffer, const u32 offset) noexcept
{
    commands::PushConstantsCommand cmd {
        .ubo_buffer = ubo_buffer,
        .offset = offset,
    };

    this->construct_command(core::move(cmd));
}

void CommandEncoder::bind_graphics_pipeline(const GraphicsPipelineHandle pipeline) noexcept
{
    this->construct_command(commands::BindGraphicsPipelineCommand {
        .pipeline = pipeline,
    });
}

void CommandEncoder::set_viewport(const Viewport& viewport) noexcept
{
    this->construct_command(commands::SetViewportCommand {
        .viewport = viewport,
    });
}

void CommandEncoder::set_scissor(const Scissor& scissor) noexcept
{
    this->construct_command(commands::SetScissorCommand {
        .scissor = scissor,
    });
}

void CommandEncoder::set_culling_mode(const CullingMode culling_mode) noexcept
{
    this->construct_command(commands::SetCullingModeCommand {
        .culling_mode = culling_mode,
    });
}

void CommandEncoder::bind_index_buffer(
    const BufferHandle buffer, const u64 offset, const IndexType index_type) noexcept
{
    this->construct_command(commands::BindIndexBufferCommand {
        .buffer = buffer,
        .offset = offset,
        .index_type = index_type,
    });
}

void CommandEncoder::draw(const u32 vertex_count, const u32 first_vertex) noexcept
{
    this->construct_command(commands::DrawCommand {
        .vertex_count = vertex_count,
        .first_vertex = first_vertex,
    });
}

void CommandEncoder::draw_indexed(
    const u32 indices_count, const u32 first_index, const i32 vertex_offset) noexcept
{
    this->construct_command(commands::DrawIndexedCommand {
        .indices_count = indices_count,
        .first_index = first_index,
        .vertex_offset = vertex_offset,
    });
}

void CommandEncoder::draw_indexed_instanced(
    const u32 indices_count,
    const u32 num_instances,
    const u32 first_index,
    const i32 vertex_offset,
    const u32 first_instance) noexcept
{
    this->construct_command(commands::DrawIndexedInstancedCommand {
        .indices_count = indices_count,
        .num_instances = num_instances,
        .first_index = first_index,
        .vertex_offset = vertex_offset,
        .first_instance = first_instance,
    });
}

void CommandEncoder::draw_indexed_indirect(
    const BufferHandle buffer,
    const u64 offset,
    const u32 draw_count,
    const u32 stride) noexcept
{
    this->construct_command(commands::DrawIndexedIndirectCommand {
        .buffer = buffer,
        .offset = offset,
        .draw_count = draw_count,
        .stride = stride,
    });
}

void CommandEncoder::draw_indexed_indirect_count(
    const BufferHandle buffer,
    const u64 offset,
    const BufferHandle count_buffer,
    const u64 count_buffer_offset,
    const u32 max_draw_count,
    const u32 stride) noexcept
{
    this->construct_command(commands::DrawIndexedIndirectCountCommand {
        .buffer = buffer,
        .offset = offset,
        .count_buffer = count_buffer,
        .count_buffer_offset = count_buffer_offset,
        .max_draw_count = max_draw_count,
        .stride = stride,
    });
}

void CommandEncoder::dispatch(
    const ComputePipelineHandle pipeline,
    const u32 group_count_x,
    const u32 group_count_y,
    const u32 group_count_z) noexcept
{
    this->construct_command(commands::DispatchCommand {
        .pipeline = pipeline,
        .group_count_x = group_count_x,
        .group_count_y = group_count_y,
        .group_count_z = group_count_z,
    });
}

void CommandEncoder::dispatch_indirect(
    const ComputePipelineHandle pipeline,
    const BufferHandle buffer,
    const u64 offset) noexcept
{
    this->construct_command(commands::DispatchIndirectCommand {
        .pipeline = pipeline,
        .buffer = buffer,
        .offset = offset,
    });
}

void CommandEncoder::buffer_copy(
    const BufferHandle src,
    const BufferHandle dst,
    core::Array<BufferCopyRegion> regions) noexcept
{
    this->construct_command(commands::BufferCopyCommand {
        .src = src,
        .dst = dst,
        .regions = core::move(regions),
    });
}

void CommandEncoder::texture_copy(
    const TextureHandle src,
    const AccessFlags src_texture_access,
    const TextureHandle dst,
    const AccessFlags dst_texture_access,
    core::Array<TextureCopyRegion> regions) noexcept
{
    this->construct_command(commands::TextureCopyCommand {
        .src = src,
        .src_texture_access = src_texture_access,
        .dst = dst,
        .dst_texture_access = dst_texture_access,
        .regions = core::move(regions),
    });
}

void CommandEncoder::copy_buffer_to_texture(
    const BufferHandle src,
    const TextureHandle dst,
    core::Array<BufferTextureCopyRegion> regions) noexcept
{
    this->construct_command(commands::BufferTextureCopyCommand {
        .src = src,
        .dst = dst,
        .regions = core::move(regions),
    });
}

void CommandEncoder::copy_texture_to_buffer(
    const TextureHandle src,
    const BufferHandle dst,
    core::Array<BufferTextureCopyRegion> regions) noexcept
{
    this->construct_command(commands::TextureBufferCopyCommand {
        .src = src,
        .dst = dst,
        .regions = core::move(regions),
    });
}

void CommandEncoder::global_barrier(const GlobalBarrier& barrier) noexcept
{
    this->construct_command(commands::GlobalBarrierCommand {
        .barrier = barrier,
    });
}

void CommandEncoder::texture_barrier(core::Array<TextureBarrier> barriers) noexcept
{
    this->construct_command(commands::TextureBarrierCommand {
        .barriers = core::move(barriers),
    });
}

void CommandEncoder::buffer_barrier(core::Array<BufferBarrier> barriers) noexcept
{
    this->construct_command(commands::BufferBarrierCommand {
        .barriers = core::move(barriers),
    });
}

template <typename T>
void destroy_command(T& command) noexcept
{
    command.~T();
}

void CommandEncoder::reset() noexcept
{
    if (this->has_commands()) {
        commands::BaseCommand* TNDR_RESTRICT command = m_root_command;
        [[likely]] while (command != nullptr)
        {
            switch (command->get_type()) {
#define CASE(e)                                                                          \
    case commands::CommandType::e: {                                                     \
        auto& cmd = static_cast<commands::TNDR_APPEND(e, Command)&>(*command);           \
        destroy_command(cmd);                                                            \
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

    m_command_allocator.reset();
    m_root_command = static_cast<commands::BaseCommand*>(
        m_command_allocator.get_buffer());
    m_command_link = &m_root_command;
    m_num_commands = 0;

    m_state = decltype(m_state) {};
}

bool CommandEncoder::has_commands() const noexcept
{
    return m_num_commands > 0;
}

template <typename Cmd>
void CommandEncoder::construct_command(Cmd&& cmd) noexcept
{
    using T = std::decay_t<Cmd>;

    void* const ptr = m_command_allocator.alloc(sizeof(T), alignof(T));
    T* const command = new (ptr) T(core::move(cmd));
    m_num_commands += 1;
    *m_command_link = command;
    m_command_link = &command->next;
}

} // namespace tundra::rhi
