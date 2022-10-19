#include "renderer/renderer.h"
#include "core/std/panic.h"
#include "renderer/hardware/hardware_rasterizer.h"
#include "renderer/software/software_rasterizer.h"

namespace tundra::renderer {

[[nodiscard]] RenderOutput render(
    const RendererType renderer_type,
    frame_graph::FrameGraph& fg,
    const RenderInput& input) noexcept
{
    switch (renderer_type) {
        case RendererType::Hardware: {
            return hardware::hardware_rasterizer(fg, input);
        }
        case RendererType::Software: {
            return software::software_rasterizer(fg, input);
        }
        default: {
            core::unreachable();
        }
    }
}

} // namespace tundra::renderer
