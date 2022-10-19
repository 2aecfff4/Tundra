#pragma once
#include "rhi/rhi_export.h"
#include "core/std/containers/string.h"
#include "platform/window_native_handle.h"

namespace tundra::rhi {

///
struct RHI_API SwapchainCreateInfo {
    platform::WindowNativeHandle window_handle;
    core::String name;
};

} // namespace tundra::rhi
