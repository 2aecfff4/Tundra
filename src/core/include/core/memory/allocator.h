#pragma once
#include "core/core_export.h"
#include "core/core.h"
#include <cstddef>

namespace tundra::core::memory {

/// Base class of all allocators.
class CORE_API IAllocator {
public:
    IAllocator() = default;
    virtual ~IAllocator() = default;

public:
    [[nodiscard]] virtual void* alloc(
        const usize size, const usize alignment = alignof(std::max_align_t)) = 0;
    virtual void free(void* const ptr) = 0;
};

} // namespace tundra::core::memory
