#pragma once
#include "core/core.h"
#include "core/std/shared_ptr.h"
#include "math/vector2.h"
#include "renderer/frame_graph/frame_graph.h"
#include "renderer/render_input_output.h"
#include "renderer/ubo.h"

namespace tundra::renderer::passes {

///
struct GpuRasterizerInitInput {
public:
    core::SharedPtr<UboBuffer> ubo_buffer;

public:
    math::UVec2 view_size = math::UVec2 {};

public:
    frame_graph::BufferHandle visible_meshlets_count;

public:
    const ComputePipelinesMap& compute_pipelines;
};

///
struct GpuRasterizerInitOutput {
    frame_graph::TextureHandle vis_texture;
    frame_graph::BufferHandle gpu_rasterizer_dispatch_args;
};

///
[[nodiscard]] GpuRasterizerInitOutput gpu_rasterizer_init_pass(
    frame_graph::FrameGraph& fg, const GpuRasterizerInitInput& input) noexcept;

} // namespace tundra::renderer::passes
