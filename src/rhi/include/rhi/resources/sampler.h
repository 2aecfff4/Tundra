#pragma once
#include "rhi/rhi_export.h"
#include "core/core.h"
#include "core/std/containers/string.h"
#include "rhi/resources/graphics_pipeline.h"

namespace tundra::rhi {

/// @see https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkSamplerAddressMode.html
enum class SamplerAddressMode : u8 {
    Repeat,
    MirroredRepeat,
    ClampToEdge,
    ClampToBorder,
    MirrorClampToEdge,
};

/// @see https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkFilter.html
/// @see https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#textures-texel-filtering
enum class SamplerFilter : u8 {
    /// Specifies nearest filtering.
    Nearest,
    /// Specifies linear filtering.
    Linear,
};

///
struct RHI_API SamplerCreateInfo {
    f32 anisotropy_level = 16.f;
    f32 mip_bias = 0.f;
    f32 min_mip_level = 0.f;
    f32 max_mip_level = 16.f;
    SamplerAddressMode address_mode_u = SamplerAddressMode::Repeat;
    SamplerAddressMode address_mode_v = SamplerAddressMode::Repeat;
    SamplerAddressMode address_mode_w = SamplerAddressMode::Repeat;
    SamplerFilter mag_filter = SamplerFilter::Nearest;
    SamplerFilter min_filter = SamplerFilter::Nearest;
    CompareOp sampler_compare_op = CompareOp::GreaterOrEqual;
    core::String name;
};

} // namespace tundra::rhi
