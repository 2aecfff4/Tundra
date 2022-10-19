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
struct GpuRasterizerInput {
public:
    math::Mat4 world_to_clip;
    math::UVec2 view_size = {};

public:
    rhi::BufferHandle mesh_descriptors;
    rhi::BufferHandle mesh_instance_transforms;

public:
    frame_graph::BufferHandle ubo_buffer;
    u64 ubo_buffer_offset = 0;

    frame_graph::BufferHandle visible_meshlets;
    frame_graph::BufferHandle dispatch_indirect_args;
    frame_graph::TextureHandle vis_texture;

public:
    const ComputePipelinesMap& compute_pipelines;
};

///
struct GpuRasterizerOutput {
    frame_graph::BufferHandle ubo_buffer;
    u64 ubo_buffer_offset = 0;

    frame_graph::TextureHandle vis_texture;
};

///
[[nodiscard]] GpuRasterizerOutput gpu_rasterizer_pass(
    frame_graph::FrameGraph& fg, const GpuRasterizerInput& input) noexcept;

} // namespace tundra::renderer::passes
