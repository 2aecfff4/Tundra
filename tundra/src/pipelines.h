#pragma once
#include "core/std/containers/array.h"
#include "core/std/variant.h"
#include "rhi/resources/graphics_pipeline.h"
#include <utility>

// #TODO: Hot reload + json/toml for pipeline description
namespace tundra::pipelines {

///
namespace common::culling {

inline constexpr const char* INSTANCE_CULLING_AND_LOD_PIPELINE_NAME =
    "visibility_buffer/common/culling/instance_culling_and_lod";

inline constexpr const char* MESHLET_CULLING_NAME =
    "visibility_buffer/common/culling/meshlet_culling";

} // namespace common::culling

///
namespace hardware {

///
namespace culling {

inline constexpr const char* CLEAR_INDEX_BUFFER_GENERATOR_DISPATCH_ARGS_NAME =
    "visibility_buffer/hardware/culling/clear_index_buffer_generator_dispatch_args";

inline constexpr const char* GENERATE_INDEX_BUFFER_GENERATOR_DISPATCH_ARGS_NAME =
    "visibility_buffer/hardware/culling/generate_index_buffer_generator_dispatch_args";

inline constexpr const char* INDEX_BUFFER_GENERATOR_NAME =
    "visibility_buffer/hardware/culling/index_buffer_generator";

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
struct Compute {};

///
struct Graphics {
    rhi::InputAssemblyState input_assembly;
    rhi::RasterizerState rasterizer_state;
    rhi::DepthStencilDesc depth_stencil;
    rhi::ColorBlendState color_blend_state;
    core::Option<rhi::MultisamplingState> multisampling_state;
};

///
using PipelineInfo = core::Variant<Compute, Graphics>;

///
[[nodiscard]] const core::Array<std::pair<const char*, PipelineInfo>>& get_pipelines();

} // namespace tundra::pipelines
