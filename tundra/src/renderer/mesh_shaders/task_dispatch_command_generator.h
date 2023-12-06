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
struct TaskDispatchCommandGeneratorInput {
public:
    core::SharedPtr<UboBuffer> ubo_buffer;

public:
    frame_graph::BufferHandle command_count;

public:
    const ComputePipelinesMap& compute_pipelines;
};

///
struct TaskDispatchCommandGeneratorOutput {
    frame_graph::BufferHandle task_shader_dispatch_args;
};

///
[[nodiscard]] TaskDispatchCommandGeneratorOutput task_dispatch_command_generator(
    frame_graph::FrameGraph& fg, const TaskDispatchCommandGeneratorInput& input) noexcept;

} // namespace tundra::renderer::mesh_shaders
