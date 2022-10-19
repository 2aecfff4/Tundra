#include "core/memory/linear_allocator.h"
#include "core/memory/pointer_math.h"
#include "core/std/assert.h"
#include "core/std/utils.h"

namespace tundra::core::memory {

LinearAllocator::LinearAllocator(const usize size_in_bytes) noexcept
    : m_buffer(new char[size_in_bytes])
    , m_size(size_in_bytes)
    , m_offset(0)
{
}

LinearAllocator::LinearAllocator(LinearAllocator&& rhs) noexcept
    : m_buffer(core::exchange(rhs.m_buffer, nullptr))
    , m_size(core::exchange(rhs.m_size, 0u))
    , m_offset(core::exchange(rhs.m_offset, 0u))
{
}

LinearAllocator& LinearAllocator::operator=(LinearAllocator&& rhs) noexcept
{
    if (&rhs != this) {
        if (m_buffer != nullptr) {
            delete[] m_buffer;
        }

        m_buffer = core::exchange(rhs.m_buffer, nullptr);
        m_size = core::exchange(rhs.m_size, 0u);
        m_offset = core::exchange(rhs.m_offset, 0u);
    }

    return *this;
}

LinearAllocator::~LinearAllocator() noexcept
{
    if (m_buffer != nullptr) {
        delete[] m_buffer;
    }
}

void* LinearAllocator::alloc(
    const usize size, const usize alignment /*= alignof(std::max_align_t)*/)
{
    char* p = pointer_math::align(pointer_math::add(m_buffer, m_offset), alignment);
    const usize new_offset = usize(
        static_cast<char*>(                                //
            pointer_math::align(                           //
                pointer_math::add(                         //
                    pointer_math::add(m_buffer, m_offset), //
                    size),                                 //
                alignment)                                 //
            ) -                                            //
        static_cast<char*>(m_buffer));                     //
    tndr_assert(new_offset <= m_size, "The buffer is empty!");
    m_offset = new_offset;
    return p;
}

void LinearAllocator::free(void* const ptr)
{
    // noop
    (void)ptr;
}

void LinearAllocator::reset() noexcept
{
    m_offset = 0;
}

void* LinearAllocator::get_buffer() const noexcept
{
    return m_buffer;
}

} // namespace tundra::core::memory
