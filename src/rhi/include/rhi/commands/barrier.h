#pragma once
#include "rhi/rhi_export.h"
#include "core/core.h"
#include "core/std/option.h"
#include "rhi/enums.h"
#include "rhi/resources/access_flags.h"
#include "rhi/resources/buffer.h"
#include "rhi/resources/handle.h"
#include "rhi/resources/texture.h"

namespace tundra::rhi {

///
struct RHI_API GlobalBarrier {
    AccessFlags previous_access = AccessFlags::ANY_WRITE;
    AccessFlags next_access = AccessFlags::GENERAL;
};

///
struct RHI_API TextureBarrier {
    TextureHandle texture;
    AccessFlags previous_access = AccessFlags::ANY_WRITE;
    AccessFlags next_access = AccessFlags::GENERAL;
    core::Option<QueueType> source_queue;
    core::Option<QueueType> destination_queue;
    TextureSubresourceRange subresource_range;
    bool discard_contents = false;
};

///
struct RHI_API BufferBarrier {
    BufferHandle buffer;
    AccessFlags previous_access = AccessFlags::ANY_WRITE;
    AccessFlags next_access = AccessFlags::GENERAL;
    core::Option<QueueType> source_queue;
    core::Option<QueueType> destination_queue;
    BufferSubresourceRange subresource_range;
};

} // namespace tundra::rhi
