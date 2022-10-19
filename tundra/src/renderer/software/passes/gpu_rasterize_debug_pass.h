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
struct GpuRasterizeDebugInput {
public:
    math::UVec2 view_size;

public:
    frame_graph::BufferHandle ubo_buffer;
    u64 ubo_buffer_offset = 0;

    frame_graph::TextureHandle vis_depth;

public:
    const ComputePipelinesMap& compute_pipelines;
};

///
struct GpuRasterizeDebugOutput {
    frame_graph::BufferHandle ubo_buffer;
    u64 ubo_buffer_offset = 0;

    frame_graph::TextureHandle debug_texture;
};

///
[[nodiscard]] GpuRasterizeDebugOutput gpu_rasterize_debug_pass(
    frame_graph::FrameGraph& fg, const GpuRasterizeDebugInput& input) noexcept;

} // namespace tundra::renderer::passes
