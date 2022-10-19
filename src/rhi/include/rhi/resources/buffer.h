#pragma once
#include "rhi/rhi_export.h"
#include "core/core.h"
#include "core/std/containers/string.h"
#include "core/std/span.h"
#include "core/utils/enum_flags.h"

namespace tundra::rhi {

///
static constexpr u64 WHOLE_SIZE = ~u64(0);

/// Specifies how memory is going to be used.
enum class MemoryType : u8 {
    /// This memory type experiences the most bandwidth for the GPU, but cannot provide CPU access.
    ///
    /// Usage:
    /// - Textures
    /// - Attachments
    /// - Resources written and read by device
    GPU,
    /// This memory type has CPU access optimized for uploading to the GPU,
    /// but does not experience the maximum amount of bandwidth for the GPU.
    ///
    /// Usage:
    /// - Staging buffers.
    Upload,
    /// This memory type has CPU access optimized for reading data back from the GPU,
    /// but does not experience the maximum amount of bandwidth for the GPU.
    ///
    /// Usage:
    /// - Resources written by device, read by host
    Readback,
    /// This memory is mappable on host, and preferably fast to access by the GPU.
    ///
    /// Usage:
    /// - Resources written frequently by host and read by the GPU.
    Dynamic,
};

/// Specifies where a buffer is going to be used.
enum class BufferUsageFlags : u8 {
    TRANSFER_SOURCE = 1 << 0,
    TRANSFER_DESTINATION = 1 << 1,
    /// The buffer is used as a SRV. In Vulkan known as a storage buffer. [read only]
    SRV = 1 << 2,
    /// The buffer is used as a UAV. In Vulkan known as a storage buffer. [read/write]
    UAV = 1 << 3,
    /// The buffer is used as a CBV. In Vulkan known as a uniform buffer. [read only]
    CBV = 1 << 4,
    INDEX_BUFFER = 1 << 5,
    VERTEX_BUFFER = 1 << 6,
    INDIRECT_BUFFER = 1 << 7,
    ALL = TRANSFER_SOURCE | TRANSFER_DESTINATION | SRV | UAV | CBV | INDEX_BUFFER |
          VERTEX_BUFFER | INDIRECT_BUFFER,
};

TNDR_ENUM_CLASS_FLAGS(BufferUsageFlags)

///
struct RHI_API BufferCopyRegion {
    u64 src_offset = WHOLE_SIZE;
    u64 dst_offset = WHOLE_SIZE;
    u64 size = WHOLE_SIZE;
};

///
struct RHI_API BufferCreateInfo {
    BufferUsageFlags usage = BufferUsageFlags::SRV;
    MemoryType memory_type = MemoryType::GPU;
    u64 size = 0;
    core::String name;
};

///
struct RHI_API BufferSubresourceRange {
    u64 offset = 0;
    u64 size = WHOLE_SIZE;
};

///
struct RHI_API BufferUpdateRegion {
    core::Span<const char> src;
    u64 dst_offset = 0;
};

} // namespace tundra::rhi
