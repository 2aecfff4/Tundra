#include "globals/globals.h"
#include <atomic>

namespace tundra::globals {

rhi::IRHIContext* g_rhi_context = nullptr;
std::atomic<bool> s_is_exit_requested = false;

bool is_exit_requested() noexcept
{
    return s_is_exit_requested.load();
}

void request_exit() noexcept
{
    s_is_exit_requested.store(true);
}

} // namespace tundra::globals
