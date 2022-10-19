#pragma once
#include "core/core.h"
#include "math/matrix4.h"
#include "math/vector4.h"
#include "renderer/config.h"
#include "renderer/frame_graph/frame_graph.h"
#include "renderer/render_input_output.h"
#include <array>

namespace tundra::renderer::common::culling {

///
struct MeshletCullingInput {
public:
    std::array<math::Vec4, config::NUM_PLANES> frustum_planes;
    math::Mat4 world_to_view;
    u32 max_meshlet_count = 0;

public:
    rhi::BufferHandle mesh_descriptors;
    rhi::BufferHandle mesh_instance_transforms;

public:
    frame_graph::BufferHandle ubo_buffer;
    u64 ubo_buffer_offset = 0;

    frame_graph::BufferHandle visible_instances;
    frame_graph::BufferHandle meshlet_culling_dispatch_args;

public:
    const ComputePipelinesMap& compute_pipelines;
};

///
struct MeshletCullingOutput {
    frame_graph::BufferHandle ubo_buffer;
    u64 ubo_buffer_offset = 0;

    frame_graph::BufferHandle visible_meshlets;
    frame_graph::BufferHandle visible_meshlets_count;
};

///
[[nodiscard]] MeshletCullingOutput meshlet_culling(
    frame_graph::FrameGraph& fg, const MeshletCullingInput& input) noexcept;

} // namespace tundra::renderer::common::culling
