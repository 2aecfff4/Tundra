#pragma once
#include "rhi/rhi_export.h"
#include "core/core.h"
#include "core/std/containers/string.h"
#include "rhi/resources/handle.h"

namespace tundra::rhi {

///
struct RHI_API ComputePipelineCreateInfo {
    ShaderHandle compute_shader;
    core::String name;
};

} // namespace tundra::rhi
