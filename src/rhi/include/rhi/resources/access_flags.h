#pragma once
#include "core/core.h"
#include "core/utils/enum_flags.h"

namespace tundra::rhi {

/// A set of flags representing different access patterns for global memory barriers.
///
/// This enumeration is used to specify the intended access patterns for global memory barriers.
/// Each variant represents a specific access type, and these variants can be combined
/// using bitwise OR operations to indicate multiple access types.
///
/// These flags are used to generate correct barriers.
enum class GlobalAccessFlags : u32 {
    NONE = 0,
    TRANSFER_SOURCE = 1 << 0,
    TRANSFER_DESTINATION = 1 << 1,

    // Textures
    GRAPHICS_SAMPLED_IMAGE = 1 << 2,
    COMPUTE_SAMPLED_IMAGE = 1 << 3,
    COMPUTE_STORAGE_IMAGE_READ = 1 << 4,
    COMPUTE_STORAGE_IMAGE_WRITE = 1 << 5,
    COMPUTE_STORAGE_IMAGE = COMPUTE_STORAGE_IMAGE_READ | COMPUTE_STORAGE_IMAGE_WRITE,
    GRAPHICS_STORAGE_IMAGE_READ = 1 << 6,
    GRAPHICS_STORAGE_IMAGE_WRITE = 1 << 7,
    GRAPHICS_STORAGE_IMAGE = GRAPHICS_STORAGE_IMAGE_READ | GRAPHICS_STORAGE_IMAGE_WRITE,
    COLOR_ATTACHMENT_READ = 1 << 8,
    COLOR_ATTACHMENT_WRITE = 1 << 9,
    COLOR_ATTACHMENT = COLOR_ATTACHMENT_READ | COLOR_ATTACHMENT_WRITE,
    DEPTH_STENCIL_ATTACHMENT_READ = 1 << 10,
    DEPTH_STENCIL_ATTACHMENT_WRITE = 1 << 11,
    DEPTH_STENCIL_ATTACHMENT = DEPTH_STENCIL_ATTACHMENT_READ |
                               DEPTH_STENCIL_ATTACHMENT_WRITE,
    PRESENT = 1 << 12,

    // Buffers
    COMPUTE_STORAGE_BUFFER_READ = 1 << 13,
    COMPUTE_STORAGE_BUFFER_WRITE = 1 << 14,
    COMPUTE_STORAGE_BUFFER = COMPUTE_STORAGE_BUFFER_READ | COMPUTE_STORAGE_BUFFER_WRITE,
    GRAPHICS_STORAGE_BUFFER_READ = 1 << 15,
    GRAPHICS_STORAGE_BUFFER_WRITE = 1 << 16,
    GRAPHICS_STORAGE_BUFFER = GRAPHICS_STORAGE_BUFFER_READ |
                              GRAPHICS_STORAGE_BUFFER_WRITE,
    UNIFORM_BUFFER = 1 << 17,
    INDEX_BUFFER = 1 << 18,
    VERTEX_BUFFER = 1 << 19,
    INDIRECT_BUFFER = 1 << 20,
    ALL = 1 << 21,
};

TNDR_ENUM_CLASS_FLAGS(GlobalAccessFlags)

} // namespace tundra::rhi
