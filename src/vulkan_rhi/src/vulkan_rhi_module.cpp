#include "vulkan_rhi/vulkan_rhi_export.h"
#include "core/module/module.h"
#include "core/std/unique_ptr.h"
#include "rhi/rhi_context.h"
#include "rhi/rhi_module.h"
#include "vulkan_rhi_context.h"

namespace tundra::vulkan_rhi {

///
class VulkanRHIModule : public rhi::IRHIModule {
public:
    [[nodiscard]] virtual core::UniquePtr<rhi::IRHIContext> create_rhi() override
    {
        return core::make_unique<VulkanRHIContext>();
    }
};

} // namespace tundra::vulkan_rhi

TNDR_IMPLEMENT_MODULE(tundra::vulkan_rhi::VulkanRHIModule, "vulkan_rhi");
