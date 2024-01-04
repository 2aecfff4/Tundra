#pragma once
#include "renderer/renderer_export.h"
#include "core/core.h"
#include "core/std/option.h"
#include "renderer/frame_graph/enums.h"
#include "renderer/frame_graph/resources/enums.h"
#include "rhi/resources/access_flags.h"

namespace tundra::renderer::frame_graph {

///
using GlobalAccessFlags = rhi::GlobalAccessFlags;
using BufferAccessFlags = rhi::BufferAccessFlags;
using TextureAccessFlags = rhi::TextureAccessFlags;

///
struct RENDERER_API GlobalBarrier {
    GlobalAccessFlags previous_access = GlobalAccessFlags::ALL;
    GlobalAccessFlags next_access = GlobalAccessFlags::ALL;
};

///
struct RENDERER_API TextureBarrier {
    ResourceId texture = NULL_RESOURCE_ID;
    TextureAccessFlags previous_access = TextureAccessFlags::NONE;
    TextureAccessFlags next_access = TextureAccessFlags::NONE;
    core::Option<QueueType> source_queue;
    core::Option<QueueType> destination_queue;
    bool discard_contents = false;
};

///
struct RENDERER_API BufferBarrier {
    ResourceId buffer = NULL_RESOURCE_ID;
    BufferAccessFlags previous_access = BufferAccessFlags::NONE;
    BufferAccessFlags next_access = BufferAccessFlags::NONE;
    core::Option<QueueType> source_queue;
    core::Option<QueueType> destination_queue;
};

} // namespace tundra::renderer::frame_graph
