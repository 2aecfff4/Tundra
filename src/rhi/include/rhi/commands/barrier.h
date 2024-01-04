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
    static const GlobalBarrier FULL_BARRIER;

    GlobalAccessFlags previous_access = GlobalAccessFlags::NONE;
    GlobalAccessFlags next_access = GlobalAccessFlags::NONE;
};

///
struct RHI_API TextureBarrier {
    TextureHandle texture;
    TextureAccessFlags previous_access = TextureAccessFlags::NONE;
    TextureAccessFlags next_access = TextureAccessFlags::NONE;
    core::Option<QueueType> source_queue;
    core::Option<QueueType> destination_queue;
    TextureSubresourceRange subresource_range;
    bool discard_contents = false;
};

///
struct RHI_API BufferBarrier {
    BufferHandle buffer;
    BufferAccessFlags previous_access = BufferAccessFlags::NONE;
    BufferAccessFlags next_access = BufferAccessFlags::NONE;
    core::Option<QueueType> source_queue;
    core::Option<QueueType> destination_queue;
    BufferSubresourceRange subresource_range;
};

} // namespace tundra::rhi
