#pragma once
#include "core/core.h"
#include "core/std/option.h"
#include "core/std/shared_ptr.h"
#include "core/std/tuple.h"
#include "loader/device.h"
#include "managers/vulkan_command_buffer_manager.h"
#include "rhi/resources/index_buffer.h"
#include "rhi/resources/render_pass.h"
#include "vulkan_barrier.h"

namespace tundra::rhi {
class CommandEncoder;

namespace commands {
struct BeginCommandBufferCommand;
struct EndCommandBufferCommand;
struct BeginRenderPassCommand;
struct EndRenderPassCommand;
struct PushConstantsCommand;
struct BindGraphicsPipelineCommand;
struct SetViewportCommand;
struct SetScissorCommand;
struct SetCullingModeCommand;
struct BindIndexBufferCommand;
struct DrawCommand;
struct DrawIndexedCommand;
struct DrawIndexedInstancedCommand;
struct DrawIndexedIndirectCommand;
struct DrawIndexedIndirectCountCommand;
struct DispatchCommand;
struct DispatchIndirectCommand;
struct BufferCopyCommand;
struct TextureCopyCommand;
struct BufferTextureCopyCommand;
struct TextureBufferCopyCommand;
struct GlobalBarrierCommand;
struct TextureBarrierCommand;
struct BufferBarrierCommand;

} // namespace commands

} // namespace tundra::rhi

namespace tundra::vulkan_rhi {

class VulkanRawDevice;
struct Managers;
struct DeviceLimits;

class VulkanCommandDecoder {
private:
    const core::SharedPtr<VulkanRawDevice>& m_raw_device;
    loader::Device m_loader_device;
    VulkanCommandBufferManager::CommandBundle m_bundle;
    decltype(m_bundle.get_resources()) m_resources;
    Managers& m_managers;
    VulkanBarrier m_barrier;
    const DeviceLimits& m_device_limits;

    struct {
        rhi::GraphicsPipelineHandle current_graphics_pipeline;
        rhi::ComputePipelineHandle current_compute_pipeline;
        /// `(buffer, offset)`
        core::Tuple<rhi::BufferHandle, rhi::IndexType, u64> current_index_buffer;
        VkPipelineLayout pipeline_layout;
    } m_cache;

public:
    VulkanCommandDecoder(
        const core::SharedPtr<VulkanRawDevice>& raw_device,
        Managers& managers,
        VulkanCommandBufferManager::CommandBundle bundle) noexcept;

public:
    [[nodiscard]] VkCommandBuffer decode(const rhi::CommandEncoder& encoder) noexcept;

private:
    void begin_command_buffer(
        const rhi::commands::BeginCommandBufferCommand& cmd) noexcept;
    void end_command_buffer(const rhi::commands::EndCommandBufferCommand& cmd) noexcept;
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
    void global_barrier(const rhi::commands::GlobalBarrierCommand& cmd) noexcept;
    void texture_barrier(const rhi::commands::TextureBarrierCommand& cmd) noexcept;
    void buffer_texture_copy(const rhi::commands::BufferTextureCopyCommand& cmd) noexcept;
    void texture_buffer_copy(const rhi::commands::TextureBufferCopyCommand& cmd) noexcept;
    void buffer_barrier(const rhi::commands::BufferBarrierCommand& cmd) noexcept;

private:
    void bind_compute_pipeline(const rhi::ComputePipelineHandle pipeline) noexcept;
};

} // namespace tundra::vulkan_rhi
