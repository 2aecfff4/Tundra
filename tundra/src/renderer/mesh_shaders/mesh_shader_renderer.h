#pragma once
#include "core/core.h"
#include "renderer/render_input_output.h"

namespace tundra::renderer::frame_graph {
class FrameGraph;
} // namespace tundra::renderer::frame_graph

namespace tundra::renderer::mesh_shaders {

///
[[nodiscard]] RenderOutput mesh_shader_renderer(
    frame_graph::FrameGraph& fg, const RenderInput& input) noexcept;

} // namespace tundra::renderer::mesh_shaders
