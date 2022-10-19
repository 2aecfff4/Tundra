#pragma once
#include "core/core.h"
#include "core/std/containers/array.h"
#include "core/std/containers/hash_map.h"
#include "core/std/containers/string.h"
#include "math/matrix4.h"
#include "math/vector2.h"
#include "renderer/frame_graph/resources/handle.h"
#include "rhi/resources/handle.h"
#include "shader.h"

namespace tundra::renderer {

using ComputePipelinesMap = core::HashMap<core::String, rhi::ComputePipelineHandle>;
using GraphicsPipelinesMap = core::HashMap<core::String, rhi::GraphicsPipelineHandle>;

///
struct RenderInput {
public:
    /// View
    math::Mat4 world_to_view = math::Mat4 {};
    /// Projection
    math::Mat4 view_to_clip = math::Mat4 {};
    math::Vec3 camera_position = math::Vec3 {};
    ///
    math::UVec2 view_size = math::UVec2 {};
    /// Camera near plane.
    f32 near_plane = 0;

public:
    const core::Array<MeshInstance>& mesh_instances;
    const core::Array<MeshDescriptor>& mesh_descriptors;

    rhi::BufferHandle gpu_mesh_descriptors;
    rhi::BufferHandle gpu_mesh_instance_transforms;
    rhi::BufferHandle gpu_mesh_instances;

public:
    const ComputePipelinesMap& compute_pipelines;
    const GraphicsPipelinesMap& graphics_pipelines;
};

///
struct RenderOutput {
    frame_graph::TextureHandle color_output;
};

} // namespace tundra::renderer
