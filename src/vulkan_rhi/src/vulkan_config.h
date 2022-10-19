#pragma once
#include "core/core.h"

namespace tundra::vulkan_rhi {

inline constexpr u32 MAX_DESCRIPTOR_COUNT = (1u << 16u) - 1u;

// Bindless stuff
inline constexpr u32 NUM_BINDINGS = 4;
inline constexpr u32 BINDING_BUFFERS = 0;
inline constexpr u32 BINDING_TEXTURES = 1;
inline constexpr u32 BINDING_RW_TEXTURES = 2;
inline constexpr u32 BINDING_SAMPLERS = 3;

} // namespace tundra::vulkan_rhi
