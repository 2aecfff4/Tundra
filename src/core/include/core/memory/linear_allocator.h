#pragma once
#include "core/core_export.h"
#include "core/core.h"
#include "core/memory/allocator.h"

namespace tundra::core::memory {

/// Allocates memory linearly from a fixed chunk of memory.
class CORE_API LinearAllocator : public IAllocator {
private:
    char* m_buffer;
    [[maybe_unused]] usize m_size;
    usize m_offset;

public:
    explicit LinearAllocator(const usize size_in_bytes) noexcept;
    virtual ~LinearAllocator() noexcept override;
    LinearAllocator(LinearAllocator&& rhs) noexcept;
    LinearAllocator& operator=(LinearAllocator&& rhs) noexcept;
    LinearAllocator(const LinearAllocator&) noexcept = delete;
    LinearAllocator& operator=(const LinearAllocator&) noexcept = delete;

public:
    virtual void* alloc(
        const usize size, const usize alignment = alignof(std::max_align_t)) final;
    virtual void free(void* const ptr) final;
    void reset() noexcept;
    [[nodiscard]] void* get_buffer() const noexcept;
};

} // namespace tundra::core::memory
