#include "rhi/commands/barrier.h"

namespace tundra::rhi {

const GlobalBarrier GlobalBarrier::FULL_BARRIER = {
    .previous_access = GlobalAccessFlags::ALL,
    .next_access = GlobalAccessFlags::ALL,
};

} // namespace tundra::rhi
