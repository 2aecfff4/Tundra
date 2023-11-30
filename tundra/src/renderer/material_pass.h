#pragma once
#include "core/core.h"
#include "core/std/shared_ptr.h"
#include "math/matrix4.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "renderer/frame_graph/frame_graph.h"
#include "renderer/render_input_output.h"
#include "renderer/ubo.h"

namespace tundra::renderer::passes {

///
struct MaterialInput {
public:
    core::SharedPtr<UboBuffer> ubo_buffer;

public:
    math::Mat4 world_to_clip = math::Mat4 {};
    math::Vec3 camera_position = math::Vec3 {};
    math::Vec3 light_position = math::Vec3 {};
    math::Vec3 diffuse_color = math::Vec3 {};
    math::Vec3 light_color = math::Vec3 {};
    math::UVec2 view_size = math::UVec2 {};

public:
    rhi::BufferHandle mesh_descriptors;
    rhi::BufferHandle mesh_instance_transforms;
    frame_graph::BufferHandle visible_meshlets;
    frame_graph::TextureHandle vis_depth;

public:
    const ComputePipelinesMap& compute_pipelines;
};

///
struct MaterialOutput {
    frame_graph::TextureHandle color_texture;
};

///
[[nodiscard]] MaterialOutput material(
    frame_graph::FrameGraph& fg, const MaterialInput& input) noexcept;

} // namespace tundra::renderer::passes
