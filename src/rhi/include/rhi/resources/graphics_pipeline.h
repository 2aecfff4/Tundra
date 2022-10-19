#pragma once
#include "rhi/rhi_export.h"
#include "core/core.h"
#include "core/std/containers/array.h"
#include "core/std/containers/string.h"
#include "core/std/hash.h"
#include "core/std/option.h"
#include "core/utils/enum_flags.h"
#include "math/vector2.h"
#include "rhi/resources/texture.h"

namespace tundra::rhi {

///
struct Rect {
    math::IVec2 offset;
    math::UVec2 extent;
};

///
struct Scissor {
    math::IVec2 offset;
    math::UVec2 extent;
};

///
struct Viewport {
    /// The render area.
    Rect rect;
    /// The viewport depth limits.
    f32 depth_min = 0.f;
    /// The viewport depth limits.
    f32 depth_max = 1.f;
};

/// @see https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkBlendFactor.html
enum class BlendFactor : u8 {
    Zero,
    One,
    SourceColor,
    OneMinusSourceColor,
    DestinationColor,
    OneMinusDestinationColor,
    SourceAlpha,
    OneMinusSourceAlpha,
    DestinationAlpha,
    OneMinusDestinationAlpha,
    ConstantColor,
    OneMinusConstantColor,
    ConstantAlpha,
    OneMinusConstantAlpha,
    SourceAlphaSaturate,
    Source1Color,
    OneMinusSource1Color,
    Source1Alpha,
    OneMinusSource1Alpha,
};

/// @see https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPrimitiveTopology.html
enum class PrimitiveType : u8 {
    Point,
    Lines,
    Triangle,
};

/// @see https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPolygonMode.html
/// The triangle rendering mode.
enum class PolygonMode : u8 {
    Fill,
    Line,
    Point,
};

/// @see https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkCullModeFlagBits.html
/// The triangle facing direction used for primitive culling.
enum class CullingMode : u8 {
    None,
    Front,
    Back,
    FrontBack,
};

/// @see https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkFrontFace.html
enum class FrontFace : u8 {
    Clockwise,
    CounterClockwise,
};

/// @see https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkCompareOp.html
enum class CompareOp : u8 {
    Never,
    Less,
    Equal,
    LessOrEqual,
    Greater,
    NotEqual,
    GreaterOrEqual,
    Always,
};

/// @see https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkStencilOp.html
enum class StencilOp : u8 {
    Keep,
    Zero,
    Replace,
    IncrementAndClamp,
    DecrementAndClamp,
    Invert,
    IncrementAndWrap,
    DecrementAndWrap,
};

/// @see https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkColorComponentFlagBits.html
enum class ColorWriteMask : u8 {
    NONE = 0,
    RED = 1 << 0,
    GREEN = 1 << 1,
    BLUE = 1 << 2,
    ALPHA = 1 << 3,
    RGB = RED | GREEN | BLUE,
    RGBA = RED | GREEN | BLUE | ALPHA,
};

TNDR_ENUM_CLASS_FLAGS(ColorWriteMask)

///
struct RHI_API InputAssemblyState {
    PrimitiveType primitive_type = PrimitiveType::Triangle;
};

///
struct RHI_API DepthBias {
    /// A constant depth value added to each fragment.
    f32 const_factor = 0.f;
    /// The minimum or maximum depth bias of a fragment.
    f32 clamp = 0.f;
    /// A constant bias applied to a fragment’s slope in depth bias calculations.
    f32 slope_factor = 0.f;
};

///
struct RHI_API RasterizerState {
    /// The triangle rendering mode.
    PolygonMode polygon_mode = PolygonMode::Fill;
    /// The front-facing triangle orientation to be used for culling.
    FrontFace front_face = FrontFace::CounterClockwise;
    /// Controls whether to clamp the fragment’s depth values.
    /// @see https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#fragops-depth
    bool depth_clamp = false;
    /// Controls whether to bias fragment depth values.
    core::Option<DepthBias> depth_bias;
    /// The width of rasterized line segments.
    f32 line_width = 1.f;
};

///
struct RHI_API BlendOp {

    ///
    struct Add {
        BlendFactor src;
        BlendFactor dst;
    };

    ///
    struct Subtract {
        BlendFactor src;
        BlendFactor dst;
    };

    ///
    struct ReverseSubtract {
        BlendFactor src;
        BlendFactor dst;
    };

    ///
    struct Min {
    };

    ///
    struct Max {
    };

    ///
    using Op = core::Variant<Add, Subtract, ReverseSubtract, Min, Max>;
};

///
struct RHI_API BlendDesc {
    /// The blend operations used on the color channels(R, G, B).
    BlendOp::Op color = BlendOp::Add {
        .src = BlendFactor::One,
        .dst = BlendFactor::Zero,
    };
    /// The blend operations used on the alpha channel(A).
    BlendOp::Op alpha = BlendOp::Add {
        .src = BlendFactor::One,
        .dst = BlendFactor::Zero,
    };
};

///
struct RHI_API ColorBlendDesc {
    /// Color write mask.
    ColorWriteMask mask = ColorWriteMask::RGBA;
    /// Blend desc.
    core::Option<BlendDesc> blend;
    /// The format of color attachment.
    TextureFormat format = TextureFormat::R8_G8_B8_A8_UNORM;
};

///
struct RHI_API ColorBlendState {
    core::Array<ColorBlendDesc> attachments;
};

///
struct RHI_API DepthTest {
    /// Comparison function to use on the depth buffer.
    CompareOp op = CompareOp::GreaterOrEqual;
    /// Specifies whether to write to the depth buffer or not.
    bool write = true;
};

///
struct RHI_API StencilOpDesc {
    /// The comparison operator used in the stencil test.
    CompareOp compare_op = CompareOp::Always;
    /// The action performed on samples that fail the stencil test.
    StencilOp fail_op = StencilOp::Keep;
    /// The action performed on samples that pass both the depth and stencil tests.
    StencilOp pass_op = StencilOp::Keep;
    /// The action performed on samples that pass the stencil test and fail the depth test.
    StencilOp depth_fail_op = StencilOp::Keep;
    /// #TODO
    u32 compare_mask = ~0u;
    /// #TODO
    u32 write_mask = ~0u;
    /// #TODO
    u32 reference = 0;
};

///
struct RHI_API StencilTest {
    StencilOpDesc front;
    StencilOpDesc back;
};

///
struct RHI_API DepthStencilDesc {
    /// Optional depth testing/writing.
    core::Option<DepthTest> depth_test = DepthTest {};
    /// Enable depth bounds testing.
    bool depth_bounds = false;
    /// Optional stencil test/write.
    core::Option<StencilTest> stencil_test;
    /// The format of depth/stencil attachment.
    TextureFormat format = TextureFormat::D32_FLOAT_S8_UINT;
};

///
struct RHI_API MultisamplingState {
    /// The number of samples used in rasterization.
    SampleCount sample_count = SampleCount::Count1;
    /// Used for sample shading.
    /// @see https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#primsrast-sampleshading
    core::Option<f32> sample_shading;
    /// @see https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#fragops-covg
    bool alpha_coverage = false;
    /// @see https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#fragops-covg
    bool alpha_to_one = false;

    [[nodiscard]] bool operator==(const MultisamplingState&) const noexcept = default;
    [[nodiscard]] bool operator!=(const MultisamplingState&) const noexcept = default;
};

///
struct RHI_API GraphicsPipelineCreateInfo {
    InputAssemblyState input_assembly;
    RasterizerState rasterizer_state;
    DepthStencilDesc depth_stencil;
    ColorBlendState color_blend_state;
    core::Option<MultisamplingState> multisampling_state;
    ShaderHandle vertex_shader;
    ShaderHandle fragment_shader;
    core::String name;
};

} // namespace tundra::rhi
