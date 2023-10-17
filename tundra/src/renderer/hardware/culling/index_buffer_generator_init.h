#pragma once
#include "core/core.h"
#include "core/std/shared_ptr.h"
#include "renderer/frame_graph/frame_graph.h"
#include "renderer/render_input_output.h"
#include "renderer/ubo.h"

namespace tundra::renderer::hardware::culling {

///
struct IndexBufferGeneratorInitInput {
public:
    core::SharedPtr<UboBuffer> ubo_buffer;

public:
    u32 num_loops = 0;
    frame_graph::BufferHandle num_visible_meshlets;

public:

public:
    const ComputePipelinesMap& compute_pipelines;
};

///
struct IndexBufferGeneratorInitOutput {
    frame_graph::BufferHandle meshlet_offsets;
    frame_graph::BufferHandle index_buffer;
    frame_graph::BufferHandle visible_indices_count;

    frame_graph::BufferHandle draw_meshlets_draw_args;
    frame_graph::BufferHandle draw_count;
    frame_graph::BufferHandle index_generator_dispatch_indirect_commands;
};

///
[[nodiscard]] IndexBufferGeneratorInitOutput index_buffer_generator_init(
    frame_graph::FrameGraph& fg, //
    const IndexBufferGeneratorInitInput& input) noexcept;

} // namespace tundra::renderer::hardware::culling
