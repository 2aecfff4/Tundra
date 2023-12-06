#pragma once
#include "core/core.h"
#include "core/std/shared_ptr.h"
#include "math/vector4.h"
#include "renderer/config.h"
#include "renderer/frame_graph/frame_graph.h"
#include "renderer/frame_graph/resources/handle.h"
#include "renderer/render_input_output.h"
#include "renderer/ubo.h"
#include <array>

namespace tundra::renderer::mesh_shaders {

///
struct MeshShaderInput {
public:
    core::SharedPtr<UboBuffer> ubo_buffer;

public:
    math::UVec2 view_size = math::UVec2 {};
    math::Mat4 view_to_clip = math::Mat4 {};
    math::Mat4 world_to_view = math::Mat4 {};
    std::array<math::Vec4, config::NUM_PLANES> frustum_planes = {};
    math::Vec3 camera_position = {};

public:
    rhi::BufferHandle mesh_descriptors;
    rhi::BufferHandle mesh_instances;
    rhi::BufferHandle mesh_instance_transforms;

    frame_graph::BufferHandle command_count;
    frame_graph::BufferHandle command_buffer;
    frame_graph::BufferHandle task_shader_dispatch_args;

public:
    const GraphicsPipelinesMap& graphics_pipelines;
};

///
struct MeshShaderOutput {
    frame_graph::TextureHandle visibility_buffer;
    frame_graph::TextureHandle depth_buffer;
};

///
[[nodiscard]] MeshShaderOutput mesh_shader(
    frame_graph::FrameGraph& fg, const MeshShaderInput& input) noexcept;

} // namespace tundra::renderer::mesh_shaders
