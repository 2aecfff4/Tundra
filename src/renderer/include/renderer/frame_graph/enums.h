#pragma once
#include "core/core.h"

namespace tundra::renderer::frame_graph {

///
enum class QueueType : u8 {
    Graphics,
    /// Don't use compute queue for small work. It's most likely not worth it.
    /// Use it for long async tasks and post-processing at the end of frame.
    AsyncCompute,
    Transfer,
    Present,
};

} // namespace tundra::renderer::frame_graph
