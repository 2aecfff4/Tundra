#pragma once
#include "core/core.h"
#include "math/matrix4.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "math/vector4.h"
#include "renderer/frame_graph/frame_graph.h"
#include "renderer/renderer.h"

namespace tundra::renderer::passes {

///
struct GpuRasterizerInitInput {
public:
    math::UVec2 view_size;

public:
    frame_graph::BufferHandle ubo_buffer;
    u64 ubo_buffer_offset = 0;

    frame_graph::BufferHandle visible_meshlets_count;

public:
    const ComputePipelinesMap& compute_pipelines;
};

///
struct GpuRasterizerInitOutput {
    frame_graph::BufferHandle ubo_buffer;
    u64 ubo_buffer_offset = 0;

    frame_graph::TextureHandle vis_texture;
    frame_graph::BufferHandle gpu_rasterizer_dispatch_args;
};

///
[[nodiscard]] GpuRasterizerInitOutput gpu_rasterizer_init_pass(
    frame_graph::FrameGraph& fg, const GpuRasterizerInitInput& input) noexcept;

} // namespace tundra::renderer::passes
