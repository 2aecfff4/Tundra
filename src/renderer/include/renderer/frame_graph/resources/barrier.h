#pragma once
#include "renderer/renderer_export.h"
#include "core/core.h"
#include "core/std/option.h"
#include "renderer/frame_graph/enums.h"
#include "renderer/frame_graph/resources/enums.h"
#include "rhi/resources/access_flags.h"

namespace tundra::renderer::frame_graph {

///
using AccessFlags = rhi::AccessFlags;

///
struct RENDERER_API GlobalBarrier {
    AccessFlags previous_access = AccessFlags::ANY_WRITE;
    AccessFlags next_access = AccessFlags::GENERAL;
};

///
struct RENDERER_API TextureBarrier {
    ResourceId texture = NULL_RESOURCE_ID;
    AccessFlags previous_access = AccessFlags::ANY_WRITE;
    AccessFlags next_access = AccessFlags::GENERAL;
    core::Option<QueueType> source_queue;
    core::Option<QueueType> destination_queue;
    bool discard_contents = false;
};

///
struct RENDERER_API BufferBarrier {
    ResourceId buffer = NULL_RESOURCE_ID;
    AccessFlags previous_access = AccessFlags::ANY_WRITE;
    AccessFlags next_access = AccessFlags::GENERAL;
    core::Option<QueueType> source_queue;
    core::Option<QueueType> destination_queue;
};

} // namespace tundra::renderer::frame_graph
