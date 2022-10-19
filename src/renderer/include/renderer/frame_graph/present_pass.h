#pragma once
#include "renderer/renderer_export.h"
#include "core/core.h"
#include "core/std/option.h"
#include "renderer/frame_graph/resources/barrier.h"
#include "renderer/frame_graph/resources/handle.h"
#include "rhi/resources/handle.h"

namespace tundra::renderer::frame_graph {

///
struct RENDERER_API PresentPass {
    rhi::SwapchainHandle swapchain;
    TextureHandle texture;
    core::Option<TextureBarrier> barrier;

    static constexpr AccessFlags ACCESS_FLAGS = rhi::AccessFlags::TRANSFER_READ;
};

} // namespace tundra::renderer::frame_graph
