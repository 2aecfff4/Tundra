#pragma once
#include "renderer/renderer_export.h"
#include "core/core.h"
#include "renderer/frame_graph2/resources/usage_flags.h"

namespace tundra::renderer::frame_graph2 {

///
class RENDERER_API Edge {
private:
    ResourceUsage m_resource_usage;

public:
    explicit Edge(const ResourceUsage resource_usage) noexcept;

public:
    [[nodiscard]]
    ResourceUsage resource_usage() const noexcept
    {
        return m_resource_usage;
    }

    [[nodiscard]] bool is_write() const noexcept { return false; }
};

} // namespace tundra::renderer::frame_graph2
