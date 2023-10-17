#pragma once
#include "core/core.h"
#include "core/std/shared_ptr.h"
#include "math/matrix4.h"
#include "math/vector2.h"
#include "renderer/frame_graph/frame_graph.h"
#include "renderer/render_input_output.h"
#include "renderer/ubo.h"

namespace tundra::renderer::passes {

///
struct GpuRasterizerInput {
public:
    core::SharedPtr<UboBuffer> ubo_buffer;

public:
    math::Mat4 world_to_clip = math::Mat4 {};
    math::UVec2 view_size = math::UVec2 {};

public:
    rhi::BufferHandle mesh_descriptors;
    rhi::BufferHandle mesh_instance_transforms;

public:
    frame_graph::BufferHandle visible_meshlets;
    frame_graph::BufferHandle dispatch_indirect_args;
    frame_graph::TextureHandle vis_texture;

public:
    const ComputePipelinesMap& compute_pipelines;
};

///
struct GpuRasterizerOutput {
    frame_graph::TextureHandle vis_texture;
};

///
[[nodiscard]] GpuRasterizerOutput gpu_rasterizer_pass(
    frame_graph::FrameGraph& fg, const GpuRasterizerInput& input) noexcept;

} // namespace tundra::renderer::passes
