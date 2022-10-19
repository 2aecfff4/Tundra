#pragma once
#include "renderer/renderer_export.h"
#include "core/core.h"
#include "core/std/containers/hash_map.h"
#include "renderer/frame_graph/resources/handle.h"
#include "rhi/resources/handle.h"

namespace tundra::renderer::frame_graph {

///
class RENDERER_API Registry {
private:
    core::HashMap<TextureHandle, rhi::TextureHandle> m_textures;
    core::HashMap<BufferHandle, rhi::BufferHandle> m_buffers;

public:
    void add_texture(
        const TextureHandle fg_handle, const rhi::TextureHandle rhi_handle) noexcept;
    void add_buffer(
        const BufferHandle fg_handle, const rhi::BufferHandle rhi_handle) noexcept;

private:
    void clear() noexcept;

public:
    [[nodiscard]] rhi::TextureHandle get_texture(
        const TextureHandle fg_handle) const noexcept;
    [[nodiscard]] rhi::BufferHandle get_buffer(
        const BufferHandle fg_handle) const noexcept;

private:
    friend class FrameGraph;
};

} // namespace tundra::renderer::frame_graph
