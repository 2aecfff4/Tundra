#pragma once
#include "core/std/containers/array.h"
#include "core/std/containers/string.h"
#include "core/std/variant.h"
#include "rhi/resources/graphics_pipeline.h"
#include <utility>

// #TODO: Hot reload + json/toml for pipeline description
namespace tundra::pipelines {

///
namespace common::culling {

inline constexpr const char* INSTANCE_CULLING_AND_LOD_PIPELINE_INIT_NAME =
    "visibility_buffer/common/culling/instance_culling_and_lod_init";

inline constexpr const char* INSTANCE_CULLING_AND_LOD_PIPELINE_NAME =
    "visibility_buffer/common/culling/instance_culling_and_lod";

inline constexpr const char* MESHLET_CULLING_INIT_NAME =
    "visibility_buffer/common/culling/meshlet_culling_init";

inline constexpr const char* MESHLET_CULLING_NAME =
    "visibility_buffer/common/culling/meshlet_culling";

} // namespace common::culling

///
namespace hardware {

///
namespace culling {

inline constexpr const char* INDEX_BUFFER_GENERATOR_INIT_NAME =
    "visibility_buffer/hardware/culling/index_buffer_generator_init";

inline constexpr const char* INDEX_BUFFER_GENERATOR_CLEAR_NAME =
    "visibility_buffer/hardware/culling/index_buffer_generator_clear";

inline constexpr const char* INDEX_BUFFER_GENERATOR_NAME =
    "visibility_buffer/hardware/culling/index_buffer_generator";

inline constexpr const char* GENERATE_DRAW_INDIRECT_COMMANDS_NAME =
    "visibility_buffer/hardware/culling/generate_draw_indirect_commands";

} // namespace culling

///
namespace passes {

inline constexpr const char* VISIBILITY_BUFFER_PASS =
    "visibility_buffer/hardware/passes/visibility_buffer_pass";

} // namespace passes

} // namespace hardware

///
namespace software::passes {

inline constexpr const char* GPU_RASTERIZE_INIT_NAME =
    "visibility_buffer/software/passes/gpu_rasterize_init";

inline constexpr const char* GPU_RASTERIZE_PASS_NAME =
    "visibility_buffer/software/passes/gpu_rasterize";

inline constexpr const char* GPU_RASTERIZE_DEBUG_PASS_NAME =
    "visibility_buffer/software/passes/gpu_rasterize_debug";

} // namespace software::passes

///
namespace mesh_shaders::passes {

inline constexpr const char* INSTANCE_CULLING_INIT_NAME =
    "visibility_buffer/mesh_shaders/instance_culling_init";

inline constexpr const char* INSTANCE_CULLING_NAME =
    "visibility_buffer/mesh_shaders/instance_culling";

inline constexpr const char* TASK_DISPATCH_COMMAND_GENERATOR_NAME =
    "visibility_buffer/mesh_shaders/task_dispatch_command_generator";

inline constexpr const char* MESH_SHADER_NAME =
    "visibility_buffer/mesh_shaders/mesh_shader";

} // namespace mesh_shaders::passes

namespace passes {

inline constexpr const char* MATERIAL_PASS_NAME = "visibility_buffer/passes/material";

} // namespace passes

///
struct Compute {};

struct GraphicShaders {
    ///
    struct VertexShaders {
        core::String vertex_shader;
        core::Option<core::String> fragment_shader;
    };

    ///
    struct MeshShaders {
        core::Option<core::String> task_shader;
        core::String mesh_shader;
        core::Option<core::String> fragment_shader;
    };

    using Type = core::Variant<VertexShaders, MeshShaders>;
};

///
struct Graphics {
    rhi::InputAssemblyState input_assembly;
    rhi::RasterizerState rasterizer_state;
    rhi::DepthStencilDesc depth_stencil;
    rhi::ColorBlendState color_blend_state;
    core::Option<rhi::MultisamplingState> multisampling_state;
    GraphicShaders::Type shaders;
};

///
using PipelineInfo = core::Variant<Compute, Graphics>;

///
[[nodiscard]] const core::Array<std::pair<const char*, PipelineInfo>>& get_pipelines();

} // namespace tundra::pipelines
