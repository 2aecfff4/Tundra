#pragma once
#include "core/core.h"
#include "core/std/containers/array.h"
#include "core/std/containers/hash_map.h"
#include "core/std/containers/string.h"
#include "math/matrix4.h"
#include "math/range.h"
#include "math/vector2.h"
#include "renderer/frame_graph/resources/handle.h"
#include "rhi/resources/handle.h"
#include "shader.h"

namespace tundra::renderer::frame_graph {
class FrameGraph;
} // namespace tundra::renderer::frame_graph

namespace tundra::renderer {

using ComputePipelinesMap = core::HashMap<core::String, rhi::ComputePipelineHandle>;
using GraphicsPipelinesMap = core::HashMap<core::String, rhi::GraphicsPipelineHandle>;

///
struct RenderInput {
public:

public:
    /// View
    math::Mat4 world_to_view;
    /// Projection
    math::Mat4 view_to_clip;
    math::Vec3 camera_position;
    ///
    math::UVec2 view_size;
    /// Camera near plane.
    f32 near_plane;

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

///
[[nodiscard]] RenderOutput render(
    frame_graph::FrameGraph& fg, const RenderInput& input) noexcept;

///
template <typename Map>
[[nodiscard]] auto get_pipeline(const char* name, const Map& map) noexcept
{
    const auto it = map.find(name);
    tndr_assert(it != map.end(), "");
    return it->second;
}

} // namespace tundra::renderer
