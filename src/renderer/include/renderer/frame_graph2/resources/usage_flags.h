#pragma once
#include "renderer/renderer_export.h"
#include "core/core.h"
#include "core/utils/enum_flags.h"
#include "rhi/resources/access_flags.h"
#include "rhi/resources/buffer.h"
#include "rhi/resources/texture.h"

namespace tundra::renderer::frame_graph2 {

///
enum class BufferResourceUsage : u16 {
    /// No access.
    NONE = 0,
    TRANSFER = 1 << 1,
    COMPUTE_STORAGE_BUFFER = 1 << 2,
    GRAPHICS_STORAGE_BUFFER = 1 << 3,
    UNIFORM_BUFFER = 1 << 4,
    INDEX_BUFFER = 1 << 5,
    VERTEX_BUFFER = 1 << 6,
    INDIRECT_BUFFER = 1 << 7,
};
TNDR_ENUM_CLASS_FLAGS(BufferResourceUsage)

///
enum class TextureResourceUsage : u16 {
    /// No access.
    NONE = 0,
    TRANSFER = 1 << 1,
    GRAPHICS_SAMPLED_IMAGE = 1 << 8,
    COMPUTE_SAMPLED_IMAGE = 1 << 9,
    COMPUTE_STORAGE_IMAGE = 1 << 10,
    GRAPHICS_STORAGE_IMAGE = 1 << 11,
    COLOR_ATTACHMENT = 1 << 12,
    DEPTH_STENCIL_ATTACHMENT = 1 << 13,
};
TNDR_ENUM_CLASS_FLAGS(TextureResourceUsage)

///
enum class ResourceUsage : u16 {
    /// No access.
    NONE = 0,
    TRANSFER = 1 << 1,
    // Buffers
    COMPUTE_STORAGE_BUFFER = 1 << 2,
    GRAPHICS_STORAGE_BUFFER = 1 << 3,
    UNIFORM_BUFFER = 1 << 4,
    INDEX_BUFFER = 1 << 5,
    VERTEX_BUFFER = 1 << 6,
    INDIRECT_BUFFER = 1 << 7,

    // Textures
    GRAPHICS_SAMPLED_IMAGE = 1 << 8,
    COMPUTE_SAMPLED_IMAGE = 1 << 9,
    COMPUTE_STORAGE_IMAGE = 1 << 10,
    GRAPHICS_STORAGE_IMAGE = 1 << 11,
    COLOR_ATTACHMENT = 1 << 12,
    DEPTH_STENCIL_ATTACHMENT = 1 << 13,
};
TNDR_ENUM_CLASS_FLAGS(ResourceUsage)

[[nodiscard]]
constexpr ResourceUsage to_resource_usage(
    const BufferResourceUsage resource_usage) noexcept
{
    ResourceUsage flags = ResourceUsage::NONE;

    if (contains(resource_usage, BufferResourceUsage::TRANSFER)) {
        flags |= ResourceUsage::TRANSFER;
    }
    if (contains(resource_usage, BufferResourceUsage::COMPUTE_STORAGE_BUFFER)) {
        flags |= ResourceUsage::COMPUTE_STORAGE_BUFFER;
    }
    if (contains(resource_usage, BufferResourceUsage::GRAPHICS_STORAGE_BUFFER)) {
        flags |= ResourceUsage::GRAPHICS_STORAGE_BUFFER;
    }
    if (contains(resource_usage, BufferResourceUsage::UNIFORM_BUFFER)) {
        flags |= ResourceUsage::UNIFORM_BUFFER;
    }
    if (contains(resource_usage, BufferResourceUsage::INDEX_BUFFER)) {
        flags |= ResourceUsage::INDEX_BUFFER;
    }
    if (contains(resource_usage, BufferResourceUsage::VERTEX_BUFFER)) {
        flags |= ResourceUsage::VERTEX_BUFFER;
    }
    if (contains(resource_usage, BufferResourceUsage::INDIRECT_BUFFER)) {
        flags |= ResourceUsage::INDIRECT_BUFFER;
    }

    return flags;
}

[[nodiscard]] constexpr ResourceUsage to_resource_usage(
    const TextureResourceUsage resource_usage) noexcept
{
    ResourceUsage flags = ResourceUsage::NONE;

    if (contains(resource_usage, TextureResourceUsage::TRANSFER)) {
        flags |= ResourceUsage::TRANSFER;
    }
    if (contains(resource_usage, TextureResourceUsage::GRAPHICS_SAMPLED_IMAGE)) {
        flags |= ResourceUsage::GRAPHICS_SAMPLED_IMAGE;
    }
    if (contains(resource_usage, TextureResourceUsage::COMPUTE_SAMPLED_IMAGE)) {
        flags |= ResourceUsage::COMPUTE_SAMPLED_IMAGE;
    }
    if (contains(resource_usage, TextureResourceUsage::COMPUTE_STORAGE_IMAGE)) {
        flags |= ResourceUsage::COMPUTE_STORAGE_IMAGE;
    }
    if (contains(resource_usage, TextureResourceUsage::GRAPHICS_STORAGE_IMAGE)) {
        flags |= ResourceUsage::GRAPHICS_STORAGE_IMAGE;
    }
    if (contains(resource_usage, TextureResourceUsage::COLOR_ATTACHMENT)) {
        flags |= ResourceUsage::COLOR_ATTACHMENT;
    }
    if (contains(resource_usage, TextureResourceUsage::DEPTH_STENCIL_ATTACHMENT)) {
        flags |= ResourceUsage::DEPTH_STENCIL_ATTACHMENT;
    }

    return flags;
}

[[nodiscard]] constexpr rhi::BufferAccessFlags to_buffer_access_flags(
    const ResourceUsage resource_usage, const bool write)
{
    using rhi::BufferAccessFlags;
    BufferAccessFlags flags = BufferAccessFlags::NONE;
    if (resource_usage == ResourceUsage::NONE) {
        return flags;
    }

    if (contains(resource_usage, ResourceUsage::TRANSFER)) {
        flags |= BufferAccessFlags::TRANSFER_SOURCE |
                 BufferAccessFlags::TRANSFER_DESTINATION;
    }
    if (contains(resource_usage, ResourceUsage::COMPUTE_STORAGE_BUFFER)) {
        if (write) {
            flags |= BufferAccessFlags::COMPUTE_STORAGE_BUFFER_WRITE;

        } else {
            flags |= BufferAccessFlags::COMPUTE_STORAGE_BUFFER_READ;
        }
    }
    if (contains(resource_usage, ResourceUsage::GRAPHICS_STORAGE_BUFFER)) {
        if (write) {
            flags |= BufferAccessFlags::GRAPHICS_STORAGE_BUFFER_WRITE;

        } else {
            flags |= BufferAccessFlags::GRAPHICS_STORAGE_BUFFER_READ;
        }
    }
    if (contains(resource_usage, ResourceUsage::UNIFORM_BUFFER)) {
        flags |= BufferAccessFlags::UNIFORM_BUFFER;
    }
    if (contains(resource_usage, ResourceUsage::INDEX_BUFFER)) {
        flags |= BufferAccessFlags::INDEX_BUFFER;
    }
    if (contains(resource_usage, ResourceUsage::VERTEX_BUFFER)) {
        flags |= BufferAccessFlags::VERTEX_BUFFER;
    }
    if (contains(resource_usage, ResourceUsage::INDIRECT_BUFFER)) {
        flags |= BufferAccessFlags::INDIRECT_BUFFER;
    }

    return flags;
}

[[nodiscard]] constexpr rhi::TextureAccessFlags to_texture_access_flags(
    const ResourceUsage resource_usage, const bool write)
{
    using rhi::TextureAccessFlags;
    TextureAccessFlags flags = TextureAccessFlags::NONE;
    if (resource_usage == ResourceUsage::NONE) {
        return flags;
    }

    if (contains(resource_usage, ResourceUsage::TRANSFER)) {
        flags |= TextureAccessFlags::TRANSFER_SOURCE |
                 TextureAccessFlags::TRANSFER_DESTINATION;
    }

    if (contains(resource_usage, ResourceUsage::GRAPHICS_SAMPLED_IMAGE)) {
        flags |= TextureAccessFlags::GRAPHICS_SAMPLED_IMAGE;
    }
    if (contains(resource_usage, ResourceUsage::COMPUTE_SAMPLED_IMAGE)) {
        flags |= TextureAccessFlags::COMPUTE_SAMPLED_IMAGE;
    }
    if (contains(resource_usage, ResourceUsage::COMPUTE_STORAGE_IMAGE)) {
        if (write) {
            flags |= TextureAccessFlags::COMPUTE_STORAGE_IMAGE_WRITE;
        } else {
            flags |= TextureAccessFlags::COMPUTE_STORAGE_IMAGE_READ;
        }
    }
    if (contains(resource_usage, ResourceUsage::GRAPHICS_STORAGE_IMAGE)) {
        if (write) {
            flags |= TextureAccessFlags::GRAPHICS_STORAGE_IMAGE_WRITE;
        } else {
            flags |= TextureAccessFlags::GRAPHICS_STORAGE_IMAGE_READ;
        }
    }
    if (contains(resource_usage, ResourceUsage::COLOR_ATTACHMENT)) {
        if (write) {
            flags |= TextureAccessFlags::COLOR_ATTACHMENT_WRITE;
        } else {
            flags |= TextureAccessFlags::COLOR_ATTACHMENT_READ;
        }
    }
    if (contains(resource_usage, ResourceUsage::DEPTH_STENCIL_ATTACHMENT)) {
        if (write) {
            flags |= TextureAccessFlags::DEPTH_STENCIL_ATTACHMENT_WRITE;
        } else {
            flags |= TextureAccessFlags::DEPTH_STENCIL_ATTACHMENT_READ;
        }
    }

    return flags;
}

[[nodiscard]] constexpr rhi::GlobalAccessFlags to_global_access_flags(
    const ResourceUsage resource_usage, const bool write)
{
    using rhi::GlobalAccessFlags;
    GlobalAccessFlags flags = GlobalAccessFlags::NONE;
    if (resource_usage == ResourceUsage::NONE) {
        return flags;
    }

    if (contains(resource_usage, ResourceUsage::TRANSFER)) {
        flags |= GlobalAccessFlags::TRANSFER_SOURCE |
                 GlobalAccessFlags::TRANSFER_DESTINATION;
    }
    if (contains(resource_usage, ResourceUsage::COMPUTE_STORAGE_BUFFER)) {
        if (write) {
            flags |= GlobalAccessFlags::COMPUTE_STORAGE_BUFFER_WRITE;

        } else {
            flags |= GlobalAccessFlags::COMPUTE_STORAGE_BUFFER_READ;
        }
    }
    if (contains(resource_usage, ResourceUsage::GRAPHICS_STORAGE_BUFFER)) {
        if (write) {
            flags |= GlobalAccessFlags::GRAPHICS_STORAGE_BUFFER_WRITE;

        } else {
            flags |= GlobalAccessFlags::GRAPHICS_STORAGE_BUFFER_READ;
        }
    }
    if (contains(resource_usage, ResourceUsage::UNIFORM_BUFFER)) {
        flags |= GlobalAccessFlags::UNIFORM_BUFFER;
    }
    if (contains(resource_usage, ResourceUsage::INDEX_BUFFER)) {
        flags |= GlobalAccessFlags::INDEX_BUFFER;
    }
    if (contains(resource_usage, ResourceUsage::VERTEX_BUFFER)) {
        flags |= GlobalAccessFlags::VERTEX_BUFFER;
    }
    if (contains(resource_usage, ResourceUsage::INDIRECT_BUFFER)) {
        flags |= GlobalAccessFlags::INDIRECT_BUFFER;
    }

    if (contains(resource_usage, ResourceUsage::GRAPHICS_SAMPLED_IMAGE)) {
        flags |= GlobalAccessFlags::GRAPHICS_SAMPLED_IMAGE;
    }
    if (contains(resource_usage, ResourceUsage::COMPUTE_SAMPLED_IMAGE)) {
        flags |= GlobalAccessFlags::COMPUTE_SAMPLED_IMAGE;
    }
    if (contains(resource_usage, ResourceUsage::COMPUTE_STORAGE_IMAGE)) {
        if (write) {
            flags |= GlobalAccessFlags::COMPUTE_STORAGE_IMAGE_WRITE;
        } else {
            flags |= GlobalAccessFlags::COMPUTE_STORAGE_IMAGE_READ;
        }
    }
    if (contains(resource_usage, ResourceUsage::GRAPHICS_STORAGE_IMAGE)) {
        if (write) {
            flags |= GlobalAccessFlags::GRAPHICS_STORAGE_IMAGE_WRITE;
        } else {
            flags |= GlobalAccessFlags::GRAPHICS_STORAGE_IMAGE_READ;
        }
    }
    if (contains(resource_usage, ResourceUsage::COLOR_ATTACHMENT)) {
        if (write) {
            flags |= GlobalAccessFlags::COLOR_ATTACHMENT_WRITE;
        } else {
            flags |= GlobalAccessFlags::COLOR_ATTACHMENT_READ;
        }
    }
    if (contains(resource_usage, ResourceUsage::DEPTH_STENCIL_ATTACHMENT)) {
        if (write) {
            flags |= GlobalAccessFlags::DEPTH_STENCIL_ATTACHMENT_WRITE;
        } else {
            flags |= GlobalAccessFlags::DEPTH_STENCIL_ATTACHMENT_READ;
        }
    }

    return flags;
}
} // namespace tundra::renderer::frame_graph2
