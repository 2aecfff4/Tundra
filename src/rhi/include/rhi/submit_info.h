#pragma once
#include "rhi/rhi_export.h"
#include "core/core.h"
#include "core/std/containers/array.h"
#include "rhi/commands/command_encoder.h"
#include "rhi/enums.h"

namespace tundra::rhi {

///
struct RHI_API SubmitInfo {
    core::Array<CommandEncoder> encoders;
    SynchronizationStage synchronization_stage = SynchronizationStage::NONE;
    QueueType queue_type = QueueType::Graphics;

    SubmitInfo() noexcept = default;
    ~SubmitInfo() noexcept = default;
    SubmitInfo(SubmitInfo&& rhs) noexcept = default;
    SubmitInfo& operator=(SubmitInfo&& rhs) noexcept = default;
    SubmitInfo(const SubmitInfo& rhs) noexcept = delete;
    SubmitInfo& operator=(const SubmitInfo& rhs) noexcept = delete;
};

///
struct RHI_API PresentInfo {
    SwapchainHandle swapchain;
    /// Texture type **must** be `TextureType::Texture2D`!
    TextureHandle texture;
    /// Texture previous access. Used to determine the current texture layout.
    AccessFlags texture_previous_access = AccessFlags::NONE;
};

} // namespace tundra::rhi
