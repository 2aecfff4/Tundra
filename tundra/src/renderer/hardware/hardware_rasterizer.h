#pragma once
#include "core/core.h"
#include "renderer/render_input_output.h"

namespace tundra::renderer::frame_graph {
class FrameGraph;
} // namespace tundra::renderer::frame_graph

namespace tundra::renderer::hardware {

///
[[nodiscard]] RenderOutput hardware_rasterizer(
    frame_graph::FrameGraph& fg, const RenderInput& input) noexcept;

} // namespace tundra::renderer::hardware
