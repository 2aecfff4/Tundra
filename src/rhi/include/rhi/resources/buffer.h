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
    STORAGE_BUFFER = 1 << 2,
    UNIFORM_BUFFER = 1 << 3,
    INDEX_BUFFER = 1 << 4,
    VERTEX_BUFFER = 1 << 5,
    INDIRECT_BUFFER = 1 << 6,
    ALL = TRANSFER_SOURCE | TRANSFER_DESTINATION | STORAGE_BUFFER | UNIFORM_BUFFER |
          INDEX_BUFFER | VERTEX_BUFFER | INDIRECT_BUFFER,
};

TNDR_ENUM_CLASS_FLAGS(BufferUsageFlags)

/// A set of flags representing different access patterns for buffers.
///
/// This enumeration is used to specify the intended access patterns for buffers.
/// Each variant represents a specific access type, and these variants can be combined
/// using bitwise OR operations to indicate multiple access types.
///
/// This flags are used to generate correct barriers.
enum class BufferAccessFlags : u16 {
    NONE = 0,
    TRANSFER_SOURCE = 1 << 0,
    TRANSFER_DESTINATION = 1 << 1,
    COMPUTE_STORAGE_BUFFER_READ = 1 << 2,
    COMPUTE_STORAGE_BUFFER_WRITE = 1 << 3,
    COMPUTE_STORAGE_BUFFER = COMPUTE_STORAGE_BUFFER_READ | COMPUTE_STORAGE_BUFFER_WRITE,
    GRAPHICS_STORAGE_BUFFER_READ = 1 << 4,
    GRAPHICS_STORAGE_BUFFER_WRITE = 1 << 5,
    GRAPHICS_STORAGE_BUFFER = GRAPHICS_STORAGE_BUFFER_READ |
                              GRAPHICS_STORAGE_BUFFER_WRITE,
    UNIFORM_BUFFER = 1 << 6,
    INDEX_BUFFER = 1 << 7,
    VERTEX_BUFFER = 1 << 8,
    INDIRECT_BUFFER = 1 << 9,
};

TNDR_ENUM_CLASS_FLAGS(BufferAccessFlags)

///
struct RHI_API BufferCopyRegion {
    u64 src_offset = WHOLE_SIZE;
    u64 dst_offset = WHOLE_SIZE;
    u64 size = WHOLE_SIZE;
};

///
struct RHI_API BufferCreateInfo {
    BufferUsageFlags usage = BufferUsageFlags::STORAGE_BUFFER;
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
