#pragma once
#include "rhi/rhi_export.h"
#include "core/core.h"
#include "core/std/containers/array.h"
#include "core/std/containers/string.h"
#include "rhi/commands/barrier.h"
#include "rhi/resources/graphics_pipeline.h"
#include "rhi/resources/index_buffer.h"
#include "rhi/resources/render_pass.h"

namespace tundra::rhi::commands {

///
enum class CommandType : u8 {
    BeginCommandBuffer,
    EndCommandBuffer,
    BeginRegion,
    EndRegion,
    BeginRenderPass,
    EndRenderPass,
    PushConstants,
    BindGraphicsPipeline,
    SetViewport,
    SetScissor,
    SetCullingMode,
    BindIndexBuffer,
    Draw,
    DrawIndexed,
    DrawIndexedInstanced,
    DrawIndexedIndirect,
    DrawIndexedIndirectCount,
    Dispatch,
    DispatchIndirect,
    BufferCopy,
    TextureCopy,
    BufferTextureCopy,
    TextureBufferCopy,
    GlobalBarrier,
    TextureBarrier,
    BufferBarrier,
};

///
struct RHI_API BaseCommand {
public:
    BaseCommand* next;
    CommandType type;

public:
    explicit constexpr BaseCommand(const CommandType command_type) noexcept
        : next(nullptr)
        , type(command_type)
    {
    }

    BaseCommand(BaseCommand&&) noexcept = default;
    BaseCommand& operator=(BaseCommand&&) noexcept = default;
    BaseCommand(const BaseCommand&) noexcept = delete;
    BaseCommand& operator=(const BaseCommand&) noexcept = delete;

    [[nodiscard]] CommandType get_type() const noexcept
    {
        return type;
    }
};

///
template <CommandType Type>
struct Command : public BaseCommand {
    constexpr Command() noexcept
        : BaseCommand(Type)
    {
    }
};

//////////////////////////////////////////////////////////////////////////
// Commands

struct RHI_API BeginCommandBufferCommand
    : public Command<CommandType::BeginCommandBuffer> {
};

struct RHI_API EndCommandBufferCommand : public Command<CommandType::EndCommandBuffer> {
};

struct RHI_API BeginRegionCommand : public Command<CommandType::BeginRegion> {
    core::String name;
    math::Vec4 color;
};

struct RHI_API EndRegionCommand : public Command<CommandType::EndRegion> {
};

struct RHI_API BeginRenderPassCommand : public Command<CommandType::BeginRenderPass> {
    Rect render_area;
    RenderPass render_pass;
};

struct RHI_API EndRenderPassCommand : public Command<CommandType::EndRenderPass> {
};

struct RHI_API PushConstantsCommand : public Command<CommandType::PushConstants> {
    rhi::BufferHandle ubo_buffer;
    u32 offset;
};

struct RHI_API BindGraphicsPipelineCommand
    : public Command<CommandType::BindGraphicsPipeline> {
    GraphicsPipelineHandle pipeline;
};

struct RHI_API SetViewportCommand : public Command<CommandType::SetViewport> {
    Viewport viewport;
};

struct RHI_API SetScissorCommand : public Command<CommandType::SetScissor> {
    Scissor scissor;
};

struct RHI_API SetCullingModeCommand : public Command<CommandType::SetCullingMode> {
    CullingMode culling_mode;
};

struct RHI_API BindIndexBufferCommand : public Command<CommandType::BindIndexBuffer> {
    BufferHandle buffer;
    u64 offset;
    IndexType index_type;
};

struct RHI_API DrawCommand : public Command<CommandType::Draw> {
    u32 vertex_count;
    u32 first_vertex;
};

struct RHI_API DrawIndexedCommand : public Command<CommandType::DrawIndexed> {
    u32 indices_count;
    u32 first_index;
    i32 vertex_offset;
};

struct RHI_API DrawIndexedInstancedCommand
    : public Command<CommandType::DrawIndexedInstanced> {
    u32 indices_count;
    u32 num_instances;
    u32 first_index;
    i32 vertex_offset;
    u32 first_instance;
};

struct RHI_API DrawIndexedIndirectCommand
    : public Command<CommandType::DrawIndexedIndirect> {
    BufferHandle buffer;
    u64 offset;
    u32 draw_count;
    u32 stride;
};

struct RHI_API DrawIndexedIndirectCountCommand
    : public Command<CommandType::DrawIndexedIndirectCount> {
    BufferHandle buffer;
    u64 offset;
    BufferHandle count_buffer;
    u64 count_buffer_offset;
    u32 max_draw_count;
    u32 stride;
};

struct RHI_API DispatchCommand : public Command<CommandType::Dispatch> {
    ComputePipelineHandle pipeline;
    u32 group_count_x;
    u32 group_count_y;
    u32 group_count_z;
};

struct RHI_API DispatchIndirectCommand : public Command<CommandType::DispatchIndirect> {
    ComputePipelineHandle pipeline;
    BufferHandle buffer;
    u64 offset;
};

struct RHI_API BufferCopyCommand : public Command<CommandType::BufferCopy> {
    BufferHandle src;
    BufferHandle dst;
    core::Array<BufferCopyRegion> regions;
};

struct RHI_API TextureCopyCommand : public Command<CommandType::TextureCopy> {
    TextureHandle src;
    /// Texture access. Used to determine the current texture layout.
    /// Must be `AccessFlags::TRANSFER_READ` or `AccessFlags::GENERAL`.
    AccessFlags src_texture_access = AccessFlags::GENERAL;
    TextureHandle dst;
    /// Texture access. Used to determine the current texture layout.
    /// Must be `AccessFlags::TRANSFER_WRITE` or `AccessFlags::GENERAL`.
    AccessFlags dst_texture_access = AccessFlags::GENERAL;
    core::Array<TextureCopyRegion> regions;
};

struct RHI_API BufferTextureCopyCommand : public Command<CommandType::BufferTextureCopy> {
    BufferHandle src;
    TextureHandle dst;
    /// Texture access. Used to determine the current texture layout.
    /// Must be `AccessFlags::TRANSFER_WRITE` or `AccessFlags::GENERAL`.
    AccessFlags texture_access = AccessFlags::GENERAL;
    core::Array<BufferTextureCopyRegion> regions;
};

struct RHI_API TextureBufferCopyCommand : public Command<CommandType::TextureBufferCopy> {
    TextureHandle src;
    /// Texture access. Used to determine the current texture layout.
    /// Must be `AccessFlags::TRANSFER_READ` or `AccessFlags::GENERAL`.
    AccessFlags texture_access = AccessFlags::GENERAL;
    BufferHandle dst;
    core::Array<BufferTextureCopyRegion> regions;
};

struct RHI_API GlobalBarrierCommand : public Command<CommandType::GlobalBarrier> {
    GlobalBarrier barrier;
};

struct RHI_API TextureBarrierCommand : public Command<CommandType::TextureBarrier> {
    core::Array<TextureBarrier> barriers;
};

struct RHI_API BufferBarrierCommand : public Command<CommandType::BufferBarrier> {
    core::Array<BufferBarrier> barriers;
};

} // namespace tundra::rhi::commands
