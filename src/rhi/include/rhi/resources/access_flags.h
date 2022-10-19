#pragma once
#include "core/core.h"
#include "core/utils/enum_flags.h"

namespace tundra::rhi {

///
enum class AccessFlags : u32 {
    /// No access. Used for initialization.
    NONE = 0,

    //////////////////////////////////////////////////////////////////////////
    // Reads

    /// Read as an indirect buffer for drawing or dispatch.
    INDIRECT_BUFFER = 1 << 0,
    /// Read as an index buffer for drawing.
    INDEX_BUFFER = 1 << 1,
    /// Read as an vertex buffer for drawing.
    VERTEX_BUFFER = 1 << 2,
    /// Read as an SRV in graphics pipeline.
    SRV_GRAPHICS = 1 << 3,
    /// Read as an SRV in compute pipeline.
    SRV_COMPUTE = 1 << 4,
    /// Read as the source of a transfer operation.
    TRANSFER_READ = 1 << 5,
    /// Read on the host.
    HOST_READ = 1 << 6,
    /// Read by blending/logic operations.
    COLOR_ATTACHMENT_READ = 1 << 7,
    /// Read by depth/stencil operations.
    DEPTH_STENCIL_ATTACHMENT_READ = 1 << 8,
    /// Read by presentation engine.
    PRESENT = 1 << 9,

    //////////////////////////////////////////////////////////////////////////
    // Writes

    /// Read/write as an UAV in graphics pipeline.
    UAV_GRAPHICS = 1 << 10,
    /// Read/write as an UAV in compute pipeline.
    UAV_COMPUTE = 1 << 11,
    /// Written as the destination of a transfer operation.
    TRANSFER_WRITE = 1 << 12,
    /// Written on the host.
    HOST_WRITE = 1 << 13,
    /// Written as a color attachment during rendering.
    COLOR_ATTACHMENT_WRITE = 1 << 14,
    /// Written as a depth/stencil attachment during rendering.
    DEPTH_STENCIL_ATTACHMENT_WRITE = 1 << 15,

    /// DO NOT USE. This value is used for internal stuff. Please change this value after adding more access flags.
    MAX_VALUE = 15,

    /// Covers any color attachment access.
    COLOR_ATTACHMENT = COLOR_ATTACHMENT_READ | COLOR_ATTACHMENT_WRITE,
    /// Covers any depth stencil attachment access.
    DEPTH_STENCIL_ATTACHMENT = DEPTH_STENCIL_ATTACHMENT_READ |
                               DEPTH_STENCIL_ATTACHMENT_WRITE,

    /// Covers any read access.
    ANY_READ = INDIRECT_BUFFER | INDEX_BUFFER | VERTEX_BUFFER | SRV_GRAPHICS |
               SRV_COMPUTE | TRANSFER_READ | HOST_READ | COLOR_ATTACHMENT_READ |
               DEPTH_STENCIL_ATTACHMENT_READ,
    /// Covers any write access.
    ANY_WRITE = UAV_GRAPHICS | UAV_COMPUTE | TRANSFER_WRITE | HOST_WRITE |
                COLOR_ATTACHMENT_WRITE | DEPTH_STENCIL_ATTACHMENT_WRITE,

    /// Covers any transfer operation.
    TRANSFERS = TRANSFER_READ | TRANSFER_WRITE,
    /// Covers any host access.
    HOST_ACCESS = HOST_READ | HOST_WRITE,

    /// Covers any access.
    GENERAL = ANY_READ | ANY_WRITE
};

TNDR_ENUM_CLASS_FLAGS(AccessFlags)

///
[[nodiscard]] constexpr bool is_write_access(AccessFlags flags) noexcept
{
    switch (flags) {
        case rhi::AccessFlags::UAV_GRAPHICS:
        case rhi::AccessFlags::UAV_COMPUTE:
        case rhi::AccessFlags::TRANSFER_WRITE:
        case rhi::AccessFlags::HOST_WRITE:
        case rhi::AccessFlags::COLOR_ATTACHMENT_WRITE:
        case rhi::AccessFlags::DEPTH_STENCIL_ATTACHMENT_WRITE: {
            return true;
        }
        default: {
            return false;
        }
    }
}

} // namespace tundra::rhi
