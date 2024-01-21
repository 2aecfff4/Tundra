#pragma once
#include "renderer/renderer_export.h"
#include "core/core.h"
#include "core/std/containers/hash_set.h"
#include "core/std/option.h"
#include "renderer/frame_graph2/graph.h"
#include "rhi/rhi_context.h"

namespace tundra::rhi {
class IRHIContext;
}

namespace tundra::renderer::frame_graph2 {

class Registry;

///
enum class ResourceType : u8 {
    Ref,
    Buffer,
    Texture,
    TextureView,
};

[[nodiscard]]
constexpr const char* to_string(const ResourceType resource_type) noexcept
{
    switch (resource_type) {
        case ResourceType::Ref: return "Ref";
        case ResourceType::Buffer: return "Buffer";
        case ResourceType::Texture: return "Texture";
        case ResourceType::TextureView: return "TextureView";
    }
}

template <typename T>
inline constexpr ResourceType resource_type = ResourceType { 0xff };

///
class RENDERER_API ResourceNode {
private:
    ResourceType m_resource_type;
    core::String m_name;
    bool m_is_imported;
    core::Option<NodeIndex> m_creator;
    core::Option<NodeIndex> m_writer;
    core::Option<NodeIndex> m_parent;
    u16 m_generation = 0;
    core::HashSet<NodeIndex> m_readers;

public:
    ResourceNode(
        const ResourceType resource_type,
        core::String name,
        const bool is_imported,
        const core::Option<NodeIndex> creator,
        const core::Option<NodeIndex> writer,
        const core::Option<NodeIndex> parent,
        const u16 generation) noexcept;

    ResourceNode(const ResourceNode&) = delete;
    ResourceNode(ResourceNode&&) = delete;
    ResourceNode& operator=(const ResourceNode&) = delete;
    ResourceNode& operator=(ResourceNode&&) = delete;
    virtual ~ResourceNode() = default;

public:
    ///
    virtual void create(rhi::IRHIContext* const context, Registry& registry) noexcept = 0;

    ///
    virtual void destroy(rhi::IRHIContext* const context) noexcept = 0;

public:
    void add_reader(const NodeIndex reader) noexcept { m_readers.insert(reader); }

public:
    [[nodiscard]] ResourceType resource_type() const noexcept { return m_resource_type; }
    [[nodiscard]] const core::String& name() const noexcept { return m_name; }
    [[nodiscard]] bool is_imported() const noexcept { return m_is_imported; }
    [[nodiscard]] const core::Option<NodeIndex>& writer() const noexcept
    {
        return m_writer;
    }
    [[nodiscard]] const core::Option<NodeIndex>& parent() const noexcept
    {
        return m_parent;
    }
    [[nodiscard]] u16 generation() const noexcept { return m_generation; }
    [[nodiscard]] const core::HashSet<NodeIndex>& readers() const noexcept
    {
        return m_readers;
    }

public:
    template <typename T>
    [[nodiscard]]
    T* as() noexcept
    {
        if (this->resource_type() == frame_graph2::resource_type<std::decay_t<T>>) {
            return static_cast<T*>(this);
        }
        return nullptr;
    }

    template <typename T>
    [[nodiscard]]
    const T* as() const noexcept
    {
        if (this->resource_type() == frame_graph2::resource_type<std::decay_t<T>>) {
            return static_cast<const T*>(this);
        }
        return nullptr;
    }
};

///
class RENDERER_API ResourceNodeRef : public ResourceNode {
private:
    /// The original resource.
    NodeIndex m_ref;

public:
    ResourceNodeRef(
        const NodeIndex ref,
        const ResourceType resource_type,
        core::String name,
        const bool is_imported,
        const core::Option<NodeIndex> creator,
        const core::Option<NodeIndex> writer,
        const core::Option<NodeIndex> parent,
        const u16 generation) noexcept;

public:
    void create(rhi::IRHIContext* const context, Registry& registry) noexcept override;
    void destroy(rhi::IRHIContext* const context) noexcept override;

public:
    [[nodiscard]] NodeIndex ref() const noexcept { return m_ref; }
};

template <>
inline constexpr ResourceType resource_type<ResourceNodeRef> = ResourceType::Ref;

} // namespace tundra::renderer::frame_graph2
