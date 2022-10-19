#pragma once
#include "renderer/renderer_export.h"
#include "core/core.h"
#include "core/std/containers/string.h"
#include "renderer/frame_graph/resources/enums.h"

namespace tundra::rhi {
class IRHIContext;
}

namespace tundra::renderer::frame_graph {

class Registry;

///
class RENDERER_API IBaseResource {
public:
    IBaseResource() noexcept = default;
    virtual ~IBaseResource() noexcept = default;
    IBaseResource(IBaseResource&&) noexcept = delete;
    IBaseResource& operator=(IBaseResource&&) noexcept = delete;
    IBaseResource(const IBaseResource&) noexcept = delete;
    IBaseResource& operator=(const IBaseResource&) noexcept = delete;

public:
    virtual void create(rhi::IRHIContext* context, Registry& registry) noexcept = 0;
    virtual void destroy(rhi::IRHIContext* context) noexcept = 0;
    [[nodiscard]] virtual const core::String& get_name() const noexcept = 0;
    [[nodiscard]] virtual ResourceType get_resource_type() const noexcept = 0;
    [[nodiscard]] virtual bool is_transient() const noexcept = 0;
};

} // namespace tundra::renderer::frame_graph
