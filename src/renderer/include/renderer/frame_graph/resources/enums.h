#pragma once
#include "core/core.h"
#include "core/utils/enum_flags.h"
#include "rhi/resources/access_flags.h"

namespace tundra::renderer::frame_graph {

///
enum class ResourceId : u32 {};

///
inline constexpr ResourceId NULL_RESOURCE_ID = ResourceId { 0xffffffff };

///
enum class ResourceType : u8 {
    Buffer,
    Texture,
};

///
enum class ResourceUsage : u8 {
    /// No access.
    NONE = 0,
    /// Used as a color attachment during rendering.
    COLOR_ATTACHMENT = 1 << 0,
    /// Used as a depth/stencil attachment during rendering.
    DEPTH_STENCIL_ATTACHMENT = 1 << 1,
    /// Used in a graphics pipeline.
    SHADER_GRAPHICS = 1 << 2,
    /// Used in a compute pipeline.
    SHADER_COMPUTE = 1 << 3,
    /// Used as an indirect buffer for drawing or dispatch.
    INDIRECT_BUFFER = 1 << 4,
    /// Used as an index buffer for drawing.
    INDEX_BUFFER = 1 << 5,
    /// Used in a transfer operations.
    TRANSFER = 1 << 6,
    // All flags
    ALL = COLOR_ATTACHMENT | DEPTH_STENCIL_ATTACHMENT | SHADER_GRAPHICS | SHADER_COMPUTE |
          INDIRECT_BUFFER | INDEX_BUFFER | TRANSFER,
};

TNDR_ENUM_CLASS_FLAGS(ResourceUsage)

///
[[nodiscard]] constexpr rhi::AccessFlags map_resource_usage(
    const ResourceUsage resource_usage, const bool write) noexcept
{
    rhi::AccessFlags flags {};
    if (write) {
        // Writes
        if (contains(resource_usage, ResourceUsage::COLOR_ATTACHMENT)) {
            flags |= rhi::AccessFlags::COLOR_ATTACHMENT_WRITE;
        }
        if (contains(resource_usage, ResourceUsage::DEPTH_STENCIL_ATTACHMENT)) {
            flags |= rhi::AccessFlags::DEPTH_STENCIL_ATTACHMENT_WRITE;
        }
        if (contains(resource_usage, ResourceUsage::SHADER_GRAPHICS)) {
            flags |= rhi::AccessFlags::UAV_GRAPHICS;
        }
        if (contains(resource_usage, ResourceUsage::SHADER_COMPUTE)) {
            flags |= rhi::AccessFlags::UAV_COMPUTE;
        }
        if (contains(resource_usage, ResourceUsage::INDIRECT_BUFFER)) {
            flags |= rhi::AccessFlags::INDIRECT_BUFFER;
        }
        if (contains(resource_usage, ResourceUsage::INDEX_BUFFER)) {
            flags |= rhi::AccessFlags::INDEX_BUFFER;
        }
        if (contains(resource_usage, ResourceUsage::TRANSFER)) {
            flags |= rhi::AccessFlags::TRANSFER_WRITE;
        }
    } else {
        // Reads
        if (contains(resource_usage, ResourceUsage::COLOR_ATTACHMENT)) {
            flags |= rhi::AccessFlags::COLOR_ATTACHMENT_READ;
        }
        if (contains(resource_usage, ResourceUsage::DEPTH_STENCIL_ATTACHMENT)) {
            flags |= rhi::AccessFlags::DEPTH_STENCIL_ATTACHMENT_READ;
        }
        if (contains(resource_usage, ResourceUsage::SHADER_GRAPHICS)) {
            flags |= rhi::AccessFlags::SRV_GRAPHICS;
        }
        if (contains(resource_usage, ResourceUsage::SHADER_COMPUTE)) {
            flags |= rhi::AccessFlags::SRV_COMPUTE;
        }
        if (contains(resource_usage, ResourceUsage::INDIRECT_BUFFER)) {
            flags |= rhi::AccessFlags::INDIRECT_BUFFER;
        }
        if (contains(resource_usage, ResourceUsage::INDEX_BUFFER)) {
            flags |= rhi::AccessFlags::INDEX_BUFFER;
        }
        if (contains(resource_usage, ResourceUsage::TRANSFER)) {
            flags |= rhi::AccessFlags::TRANSFER_READ;
        }
    }

    return flags;
}

} // namespace tundra::renderer::frame_graph
