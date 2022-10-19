#pragma once
#include "core/core.h"

namespace tundra::vulkan_rhi {

static constexpr u32 MAX_DESCRIPTOR_COUNT = (1 << 16) - 1;

// Bindless stuff
static constexpr u32 NUM_BINDINGS = 4;
static constexpr u32 BINDING_BUFFERS = 0;
static constexpr u32 BINDING_TEXTURES = 1;
static constexpr u32 BINDING_RW_TEXTURES = 2;
static constexpr u32 BINDING_SAMPLERS = 3;

} // namespace tundra::vulkan_rhi
