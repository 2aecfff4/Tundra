#pragma once
#include "renderer/renderer_export.h"
#include "core/core.h"
#include "core/std/unique_ptr.h"
#include "core/std/variant.h"
#include "renderer/frame_graph2/resources/pass_node.h"
#include "renderer/frame_graph2/resources/resource_node.h"

namespace tundra::renderer::frame_graph2 {

struct RENDERER_API Node {
public:
    using Kind = core::Variant<core::UniquePtr<PassNode>, core::UniquePtr<ResourceNode>>;

private:
    Kind m_kind;
    u16 m_ref_count = 0;
    bool m_is_uncullable = false;

public:
    Node(Kind&& kind) noexcept
        : m_kind(core::move(kind))
    {
    }

public:
    [[nodiscard]] const Kind& kind() const noexcept { return m_kind; }

    [[nodiscard]]
    bool is_culled() const noexcept
    {
        return (m_ref_count == 0) && !m_is_uncullable;
    }

    [[nodiscard]] bool is_uncullable() const noexcept { return m_is_uncullable; }

    [[nodiscard]] u16 ref_count() const noexcept { return m_ref_count; }

    void mark_uncullable() noexcept { m_is_uncullable = true; }
    void add_ref(const u16 count = 1) noexcept { m_ref_count += count; }
    void sub_ref(const u16 count = 1) noexcept { m_ref_count -= count; }
};

} // namespace tundra::renderer::frame_graph2
