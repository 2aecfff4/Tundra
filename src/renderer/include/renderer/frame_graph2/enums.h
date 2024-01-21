#pragma once
#include "renderer/renderer_export.h"
#include "core/core.h"
#include "core/std/containers/cstring.h"

namespace tundra::renderer::frame_graph2 {

///
enum class QueueType : u8 {
    Graphics,
    Compute,
    Transfer,
    Present,
};

[[nodiscard]]
constexpr const char* to_string(const QueueType queue_type) noexcept
{
    switch (queue_type) {
        case QueueType::Graphics: return "Graphics";
        case QueueType::Compute: return "Compute";
        case QueueType::Transfer: return "Transfer";
        case QueueType::Present: return "Present";
    }
}

} // namespace tundra::renderer::frame_graph2
