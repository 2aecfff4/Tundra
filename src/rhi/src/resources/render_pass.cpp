#include "rhi/resources/render_pass.h"

namespace tundra::rhi {

const AttachmentOps AttachmentOps::DONT_CARE = AttachmentOps {
    .load = AttachmentLoadOp::DontCare,
    .store = AttachmentStoreOp::DontCare,
};

const AttachmentOps AttachmentOps::INIT = AttachmentOps {
    .load = AttachmentLoadOp::Clear,
    .store = AttachmentStoreOp::Store,
};

const AttachmentOps AttachmentOps::PRESERVE = AttachmentOps {
    .load = AttachmentLoadOp::Load,
    .store = AttachmentStoreOp::Store,
};

} // namespace tundra::rhi
