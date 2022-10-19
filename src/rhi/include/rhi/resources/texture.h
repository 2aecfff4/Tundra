#pragma once
#include "rhi/rhi_export.h"
#include "core/core.h"
#include "core/std/containers/string.h"
#include "core/std/hash.h"
#include "core/std/variant.h"
#include "core/utils/enum_flags.h"
#include "rhi/resources/buffer.h"
#include "rhi/resources/handle.h"

namespace tundra::rhi {

/// @see https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkImageAspectFlagBits.html
enum class TextureAspectFlags : u8 {
    /// Color aspect.
    COLOR = 1 << 0,
    /// Depth aspect.
    DEPTH = 1 << 1,
    /// Stencil aspect.
    STENCIL = 1 << 2,
};

TNDR_ENUM_CLASS_FLAGS(TextureAspectFlags)

/// @see https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkImageTiling.html
enum class TextureTiling : u8 {
    /// Optimal tiling for GPU memory access.
    /// Texels are laid out in an implementation-dependent arrangement, for more optimal memory access.
    Optimal,
    /// Optimal for CPU read/write.
    /// Texels are laid out in memory in row-major order, possibly with some padding on each row.
    Linear,
};

/// @see https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkImageUsageFlagBits.html
enum class TextureUsageFlags : u8 {
    /// The texture is used as a color attachment.
    COLOR_ATTACHMENT = 1 << 0,
    /// The texture is used as a depth attachment.
    DEPTH_ATTACHMENT = 1 << 1,
    /// The texture is used as a stencil attachment.
    STENCIL_ATTACHMENT = 1 << 2,
    /// The texture is used as a SRV. In Vulkan known as a sampled image.
    SRV = 1 << 3,
    /// The texture is used as a UAV. In Vulkan known as a storage image.
    UAV = 1 << 4,
    /// The texture is used as a transfer source.
    TRANSFER_SOURCE = 1 << 5,
    /// The texture is used as a transfer destination.
    TRANSFER_DESTINATION = 1 << 6,
    /// The texture is used as a source in `PresentInfo`.
    PRESENT = 1 << 7,
};

TNDR_ENUM_CLASS_FLAGS(TextureUsageFlags)

///
[[nodiscard]] constexpr bool can_transfer(const TextureUsageFlags usage_flags) noexcept
{
    return intersects(
        usage_flags,
        TextureUsageFlags::TRANSFER_SOURCE | TextureUsageFlags::TRANSFER_DESTINATION);
}

/// @see https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkSampleCountFlagBits.html
enum class SampleCount : u8 {
    Count1,
    Count2,
    Count4,
    Count8,
};

/// @see https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkExtent3D.html
struct Extent {
    /// Texture width. **Must** be greater than `0`.
    u32 width = 1;
    /// Texture height. **Must** be greater than `0`.
    u32 height = 1;
    /// Texture depth. **Must** be greater than `0`.
    u32 depth = 1;

    [[nodiscard]] bool operator==(const Extent&) const noexcept = default;
    [[nodiscard]] bool operator!=(const Extent&) const noexcept = default;
};

/// An offset into a `Texture` used for texture copy operations.
struct Offset {
    /// X offset.
    i32 x = 0;
    /// Y offset.
    i32 y = 0;
    /// Z offset.
    i32 z = 0;

    [[nodiscard]] bool operator==(const Offset&) const noexcept = default;
    [[nodiscard]] bool operator!=(const Offset&) const noexcept = default;
};

/// # Format types:
/// - UNORM - Unsigned normalized format in the range of [0, 1].
/// - SNORM - Signed normalized format in the range of [-1, 1].
/// - UINT - Unsigned integer format in the range [0, 2^(n)-1].
/// - SINT - Signed integer format in the range [-2^(n-1), 2^(n-1) - 1].
/// - FLOAT - Signed floating-point format.
/// - For more see https://www.khronos.org/registry/vulkan/specs/1.2/html/vkspec.html#_identification_of_formats
///
/// # Formats supported by **any** PC GPU, with `TextureTiling::Optimal`
/// - https://vulkan.gpuinfo.org/listformats.php
/// - A1_R5_G5_B5
/// - A2_B10_G10_R10_UINT
/// - A2_B10_G10_R10_UNORM
/// - A8_B8_G8_R8_SINT
/// - A8_B8_G8_R8_SNORM
/// - A8_B8_G8_R8_UINT
/// - A8_B8_G8_R8_UNORM
/// - B10_G11_R11_FLOAT
/// - B8_G8_R8_A8_UNORM
/// - R5_G6_B5_UNORM
/// - D16_UNORM
/// - D32_FLOAT
/// - D32_FLOAT_S8_UINT
/// - R8_SINT
/// - R8_UINT
/// - R8_SNORM
/// - R8_UNORM
/// - R8_G8_SINT
/// - R8_G8_UINT
/// - R8_G8_SNORM
/// - R8_G8_UNORM
/// - R8_G8_B8_A8_SINT
/// - R8_G8_B8_A8_UINT
/// - R8_G8_B8_A8_SNORM
/// - R8_G8_B8_A8_UNORM
/// - R16_SNORM
/// - R16_UNORM
/// - R16_FLOAT
/// - R16_SINT
/// - R16_UINT
/// - R16_G16_SNORM
/// - R16_G16_UNORM
/// - R16_G16_FLOAT
/// - R16_G16_SINT
/// - R16_G16_UINT
/// - R16_G16_B16_A16_SNORM
/// - R16_G16_B16_A16_UNORM
/// - R16_G16_B16_A16_FLOAT
/// - R16_G16_B16_A16_SINT
/// - R16_G16_B16_A16_UINT
/// - R32_FLOAT
/// - R32_SINT
/// - R32_UINT
/// - R32_G32_FLOAT
/// - R32_G32_SINT
/// - R32_G32_UINT
/// - R32_G32_B32_A32_FLOAT
/// - R32_G32_B32_A32_SINT
/// - R32_G32_B32_A32_UINT
/// - BC1_RGBA_UNORM
/// - BC1_RGBA_SRGB
/// - BC2_UNORM
/// - BC2_SRGB
/// - BC3_UNORM
/// - BC3_SRGB
/// - BC4_UNORM
/// - BC4_SNORM
/// - BC5_UNORM
/// - BC5_SNORM
/// - BC7_UNORM
/// - BC7_SRGB
///
/// @see https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkFormat.html
/// @see https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkImageAspectFlagBits.html
enum class TextureFormat : u16 {
    INVALID,
#define TEXTURE_FORMAT(x, _1, _2, _3, _4) x,
#include "rhi/resources/texture_formats.inc"
#undef TEXTURE_FORMAT
};

///
struct RHI_API TextureFormatDesc {
    u16 num_bits;
    u8 block_dim; // (block_dim, block_dim)
    bool packed;
    TextureAspectFlags aspect;

    [[nodiscard]] bool is_packed() const noexcept
    {
        return packed;
    }

    [[nodiscard]] bool is_compressed() const noexcept
    {
        return block_dim != 1;
    }
};

///
[[nodiscard]] RHI_API TextureFormatDesc
    get_texture_format_desc(const TextureFormat format) noexcept;

/// Returns true whenever texture can be used in present pass.
[[nodiscard]] RHI_API bool is_valid_src_present(const TextureFormat format) noexcept;

///
struct RHI_API TextureKind {

    ///
    struct Texture1D {
        /// `width` **must** be greater than `0`.
        u32 width = 1;
        /// `num_layers` **must** be greater than `0`.
        u32 num_layers = 1;
        /// `num_mips` **must** be greater than `0`.
        u32 num_mips = 1;
    };

    ///
    struct Texture2D {
        /// `width` **must** be greater than `0`.
        u32 width = 1;
        /// `height` **must** be greater than `0`.
        u32 height = 1;
        /// `num_layers` **must** be greater than `0`.
        u32 num_layers = 1;
        /// `num_mips` **must** be greater than `0`.
        u32 num_mips = 1;
        /// When `sample_count` != [`SampleCount::Count1`], then `num_mips` **must** be equal to `1`.
        SampleCount sample_count = SampleCount::Count1;
    };

    ///
    struct Texture3D {
        /// `width` **must** be greater than `0`.
        u32 width = 1;
        /// `height` **must** be greater than `0`.
        u32 height = 1;
        /// `depth` **must** be greater than `0`.
        u32 depth = 1;
        /// `num_mips` **must** be greater than `0`.
        u32 num_mips = 1;
    };

    ///
    struct TextureCube {
        /// `width` **must** be greater than `0`.
        u32 width = 1;
        /// `height` **must** be greater than `0`.
        u32 height = 1;
        /// `num_mips` **must** be greater than `0`.
        u32 num_mips = 1;
    };

    ///
    using Kind = core::Variant<Texture1D, Texture2D, Texture3D, TextureCube>;

    [[nodiscard]] static Extent get_extent(const Kind& kind) noexcept;
    [[nodiscard]] static SampleCount get_sample_count(const Kind& kind) noexcept;
    [[nodiscard]] static u32 get_num_layers(const Kind& kind) noexcept;
    [[nodiscard]] static u32 get_num_mips(const Kind& kind) noexcept;
    [[nodiscard]] static u32 compute_num_mips(const Kind& kind) noexcept;
};

///
struct RHI_API TextureCreateInfo {
    TextureKind::Kind kind;
    TextureFormat format = TextureFormat::R8_G8_B8_A8_UNORM;
    TextureUsageFlags usage = TextureUsageFlags::SRV;
    TextureTiling tiling = TextureTiling::Optimal;
    MemoryType memory_type = MemoryType::GPU;
    core::String name;
};

///
static constexpr u32 REMAINING_ARRAY_LAYERS = ~u32(0);

///
static constexpr u32 REMAINING_MIP_LEVELS = ~u32(0);

///
struct RHI_API TextureSubresourceRange {
    /// First layer in an array texture.
    u32 first_layer = 0;
    /// Number of layers.
    u32 layer_count = REMAINING_ARRAY_LAYERS;
    /// First mip level.
    u32 first_mip_level = 0;
    /// Number of mip levels.
    u32 mip_count = REMAINING_MIP_LEVELS;
};

///
struct RHI_API TextureViewSubresource {

    ///
    struct Texture1D {
        /// Must be between 0 - `TextureCreateInfo::mip_levels`.
        u32 first_mip = 0;
        /// `first_mip` + `mip_count` must be less than `TextureCreateInfo::mip_levels`.
        u32 mip_count = 1;
    };

    ///
    struct Texture2D {
        /// Must be between 0 - `TextureCreateInfo::mip_levels`.
        u32 first_mip = 0;
        /// `first_mip` + `mip_count` must be less than `TextureCreateInfo::mip_levels`.
        u32 mip_count = 1;
    };

    ///
    struct Texture3D {
        /// Must be between 0 - `TextureCreateInfo::mip_levels`.
        u32 first_mip = 0;
        /// `first_mip` + `mip_count` must be less than `TextureCreateInfo::mip_levels`.
        u32 mip_count = 1;
        /// `first_layer` specifies the first slice in the 3D texture.
        /// Must be less than `TextureCreateInfo::depth - 1`.
        u32 first_layer = 0;
        /// `layer_count` specifies the number of slices in the volume texture.
        /// `first_layer + layer_count` must be less than `TextureCreateInfo::depth`.
        u32 layer_count = 1;
    };

    ///
    struct TextureCube {
        /// Must be between 0 - `TextureCreateInfo::mip_levels`.
        u32 first_mip = 0;
        /// `first_mip` + `mip_count` must be less than `TextureCreateInfo::mip_levels`.
        u32 mip_count = 1;
        /// `first_layer` specifies the first 2D texture in the cubemap (0 - 6).
        u32 first_layer = 0;
        /// `layer_count` specifies the number of 2D surfaces in the cubemap that the texture view should use.
        /// `first_layer + layer_count` must be less than 6.
        u32 layer_count = 1;
    };

    ///
    using Subresource = core::Variant<Texture1D, Texture2D, Texture3D, TextureCube>;
};

///
struct RHI_API TextureViewCreateInfo {
    TextureHandle texture;
    TextureViewSubresource::Subresource subresource;
    core::String name;
};

///
struct RHI_API TextureSubresourceLayers {
    /// The mipmap level to copy.
    /// Must be between 0 - `TextureCreateInfo::mip_levels`.
    u32 mip_level = 0;
    /// First layer in an array texture.
    u32 first_layer = 0;
    /// Number of layers.
    u32 layer_count = 1;
};

///
struct RHI_API TextureCopyRegion {
    /// The image subresource to copy from.
    TextureSubresourceLayers src_subresource;
    /// The source offset.
    Offset src_offset;
    /// The image subresource to copy to.
    TextureSubresourceLayers dst_subresource;
    /// The destination offset.
    Offset dst_offset;
    /// The size of the region to copy.
    Extent extent;
};

///
struct RHI_API BufferTextureCopyRegion {
    /// Buffer offset in bytes.
    u64 buffer_offset;
    /// Width of a buffer 'row' in texels.
    u32 buffer_width = 0;
    /// Height of a buffer 'image slice' in texels.
    u32 buffer_height = 0;
    /// The texture subresource.
    TextureSubresourceLayers texture_subresource;
    /// The destination offset.
    Offset texture_offset;
    /// The size of the region to copy.
    Extent texture_extent;
};

} // namespace tundra::rhi

namespace tundra::core {

template <>
struct Hash<rhi::Extent> {
    [[nodiscard]] usize operator()(const rhi::Extent& extent) const noexcept
    {
        usize seed = 0;
        core::hash_and_combine(seed, extent.width);
        core::hash_and_combine(seed, extent.height);
        core::hash_and_combine(seed, extent.depth);
        return seed;
    }
};

template <>
struct Hash<rhi::Offset> {
    [[nodiscard]] usize operator()(const rhi::Offset& offset) const noexcept
    {
        usize seed = 0;
        core::hash_and_combine(seed, offset.x);
        core::hash_and_combine(seed, offset.y);
        core::hash_and_combine(seed, offset.z);
        return seed;
    }
};

} // namespace tundra::core
