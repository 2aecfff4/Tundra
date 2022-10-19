#include "rhi/resources/texture.h"
#include "core/std/panic.h"
#include "core/std/utils.h"
#include <algorithm>

namespace tundra::rhi {

rhi::TextureFormatDesc get_texture_format_desc(const TextureFormat format) noexcept
{
    switch (format) {
#define TEXTURE_FORMAT(format, num_bits, packed, aspect, block_dim)                      \
    case TextureFormat::format:                                                          \
        return TextureFormatDesc {                                                       \
            num_bits,                                                                    \
            block_dim,                                                                   \
            packed,                                                                      \
            aspect,                                                                      \
        };

#include "rhi/resources/texture_formats.inc"

#undef TEXTURE_FORMAT

        default:
            core::panic("Invalid `TextureFormat`.");
    }
}

bool is_valid_src_present(const TextureFormat format) noexcept
{
    switch (format) {
        // 100%
        case TextureFormat::R5_G6_B5:
        case TextureFormat::A1_R5_G5_B5:
        case TextureFormat::A2_B10_G10_R10_UINT:
        case TextureFormat::A2_B10_G10_R10_UNORM:
        case TextureFormat::A8_B8_G8_R8_SINT:
        case TextureFormat::A8_B8_G8_R8_UINT:
        case TextureFormat::A8_B8_G8_R8_UNORM:
        case TextureFormat::B8_G8_R8_A8_UNORM:
        case TextureFormat::R16_FLOAT:
        case TextureFormat::R16_SINT:
        case TextureFormat::R16_UINT:
        case TextureFormat::R16_G16_FLOAT:
        case TextureFormat::R16_G16_SINT:
        case TextureFormat::R16_G16_UINT:
        case TextureFormat::R16_G16_B16_A16_FLOAT:
        case TextureFormat::R16_G16_B16_A16_SINT:
        case TextureFormat::R16_G16_B16_A16_UINT:
        case TextureFormat::R32_FLOAT:
        case TextureFormat::R32_SINT:
        case TextureFormat::R32_UINT:
        case TextureFormat::R32_G32_FLOAT:
        case TextureFormat::R32_G32_SINT:
        case TextureFormat::R32_G32_UINT:
        case TextureFormat::R32_G32_B32_A32_FLOAT:
        case TextureFormat::R32_G32_B32_A32_SINT:
        case TextureFormat::R32_G32_B32_A32_UINT:
        case TextureFormat::R8_SINT:
        case TextureFormat::R8_UINT:
        case TextureFormat::R8_UNORM:
        case TextureFormat::R8_G8_SINT:
        case TextureFormat::R8_G8_UINT:
        case TextureFormat::R8_G8_UNORM:
        case TextureFormat::R8_G8_B8_A8_SINT:
        case TextureFormat::R8_G8_B8_A8_UINT:
        case TextureFormat::R8_G8_B8_A8_UNORM:
            return true;
        // 99%
        case TextureFormat::A8_B8_G8_R8_SNORM:
        case TextureFormat::B10_G11_R11_FLOAT:
        case TextureFormat::R8_SNORM:
        case TextureFormat::R8_G8_SNORM:
            return true;
        default:
            return false;
    }
}

///////////////////////////////////////////////////////////////////////////////
// TextureKind

[[nodiscard]] Extent TextureKind::get_extent(const TextureKind::Kind& kind) noexcept
{
    return core::visit(
        core::make_overload(
            [](const Texture1D& texture) {
                return Extent {
                    .width = texture.width,
                    .height = 1,
                    .depth = 1,
                };
            },
            [](const Texture2D& texture) {
                return Extent {
                    .width = texture.width,
                    .height = texture.height,
                    .depth = 1,
                };
            },
            [](const Texture3D& texture) {
                return Extent {
                    .width = texture.width,
                    .height = texture.height,
                    .depth = texture.depth,
                };
            },
            [](const TextureCube& texture) {
                return Extent {
                    .width = texture.width,
                    .height = texture.height,
                    .depth = 1,
                };
            }),
        kind);
}

[[nodiscard]] SampleCount TextureKind::get_sample_count(
    const TextureKind::Kind& kind) noexcept
{
    return core::visit(
        core::make_overload(
            [](const Texture2D& texture) { return texture.sample_count; },
            [](const auto&) { return SampleCount::Count1; }),
        kind);
}

[[nodiscard]] u32 TextureKind::get_num_layers(const TextureKind::Kind& kind) noexcept
{
    return core::visit(
        core::make_overload(
            [](const Texture1D& texture) { return texture.num_layers; },
            [](const Texture2D& texture) { return texture.num_layers; },
            [](const Texture3D&) { return 1u; },
            [](const TextureCube&) { return 6u; }),
        kind);
}

[[nodiscard]] u32 TextureKind::get_num_mips(const TextureKind::Kind& kind) noexcept
{
    return core::visit([](const auto& t) { return t.num_mips; }, kind);
}

[[nodiscard]] u32 TextureKind::compute_num_mips(const TextureKind::Kind& kind) noexcept
{
    const auto compute_mip_level = [](const u32 max_size) {
        // Divide by 2^level.
        for (u32 level = 0; ++level;) {
            if ((max_size >> level) == 0) {
                return level;
            }
        }
        return ~0u;
    };

    return core::visit(
        core::make_overload(
            [&](const Texture2D& texture) {
                if (texture.sample_count != SampleCount::Count1) {
                    return 1u;
                } else {
                    const Extent extent = TextureKind::get_extent(texture);
                    const u32 max_size = std::max({
                        extent.width,
                        extent.height,
                        extent.depth,
                    });
                    return compute_mip_level(max_size);
                }
            },
            [&](const auto& texture) {
                const Extent extent = TextureKind::get_extent(texture);
                const u32 max_size = std::max({
                    extent.width,
                    extent.height,
                    extent.depth,
                });
                return compute_mip_level(max_size);
            }),
        kind);
}

} // namespace tundra::rhi
