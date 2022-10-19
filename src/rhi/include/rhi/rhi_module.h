#pragma once
#include "rhi/rhi_export.h"
#include "core/core.h"
#include "core/module/module.h"
#include "core/std/unique_ptr.h"
#include "rhi/rhi_context.h"

namespace tundra::rhi {

///
class RHI_API IRHIModule : public core::IModule {
public:
    /// This function creates an instance of the rhi interface.
    /// Returns a valid pointer to the rhi interface.
    [[nodiscard]] virtual core::UniquePtr<IRHIContext> create_rhi() = 0;
};

} // namespace tundra::rhi
