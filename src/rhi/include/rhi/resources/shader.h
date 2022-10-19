#pragma once
#include "rhi/rhi_export.h"
#include "core/core.h"
#include "core/std/containers/string.h"
#include "core/std/span.h"

namespace tundra::rhi {

///
enum class ShaderStage : u8 {
    VertexShader,
    FragmentShader,
    ComputeShader,
    Invalid,
};

///
struct RHI_API ShaderCreateInfo {
    ShaderStage shader_stage = ShaderStage::Invalid;
    core::Span<const char> shader_buffer;
    core::String name;
};

} // namespace tundra::rhi
