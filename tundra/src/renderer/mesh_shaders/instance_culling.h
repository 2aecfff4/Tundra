#pragma once
#include "core/core.h"
#include "core/std/shared_ptr.h"
#include "math/vector4.h"
#include "renderer/config.h"
#include "renderer/frame_graph/frame_graph.h"
#include "renderer/render_input_output.h"
#include "renderer/ubo.h"
#include <array>

namespace tundra::renderer::mesh_shaders {

///
struct InstanceCullingInput {
public:
    core::SharedPtr<UboBuffer> ubo_buffer;

public:
    std::array<math::Vec4, config::NUM_PLANES> frustum_planes = {};
    u32 num_instances = 0;

public:
    rhi::BufferHandle mesh_descriptors;
    rhi::BufferHandle mesh_instances;
    rhi::BufferHandle mesh_instance_transforms;

public:
    const ComputePipelinesMap& compute_pipelines;
};

///
struct InstanceCullingOutput {
    frame_graph::BufferHandle command_count;
    frame_graph::BufferHandle command_buffer;
};

///
[[nodiscard]] InstanceCullingOutput instance_culling(
    frame_graph::FrameGraph& fg, const InstanceCullingInput& input) noexcept;

} // namespace tundra::renderer::mesh_shaders
