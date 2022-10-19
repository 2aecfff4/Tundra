#pragma once
#include "core/core_export.h"
#include "core/core.h"

namespace tundra::core {

class CString;

namespace cstring_impl {

[[nodiscard]] constexpr u64 hash_64_fnv1a(const CString& string) noexcept;

} // namespace cstring_impl

///
class CORE_API CString {
public:
    constexpr CString(const char* begin, const usize size) noexcept
        : m_begin(begin)
        , m_size(size)
    {
    }

    constexpr CString(const char* begin, const char* end) noexcept
        : m_begin(begin)
        , m_size(static_cast<usize>(end - begin))
    {
    }

    constexpr CString(const char* begin) noexcept
        : m_begin(begin)
        , m_size(string_length(begin))
    {
    }

    template <usize N>
    constexpr CString(const char (&string)[N]) noexcept
        : m_begin(string)
        , m_size(N)
    {
    }

public:
    [[nodiscard]] constexpr static usize string_length(const char* string) noexcept
    {
        return *string != '\0' ? 1 + string_length(string + 1) : 0;
    }

    [[nodiscard]] constexpr const char* begin() const noexcept
    {
        return m_begin;
    }

    [[nodiscard]] constexpr const char* end() const noexcept
    {
        return m_begin + m_size;
    }

    [[nodiscard]] constexpr usize size() const noexcept
    {
        return m_size;
    }

    [[nodiscard]] constexpr CString substr(
        const usize begin_offset, const usize end_offset) const noexcept
    {
        return { this->begin() + begin_offset, this->end() - end_offset };
    }

    [[nodiscard]] u64 hash() const noexcept
    {
        return cstring_impl::hash_64_fnv1a(*this);
    }

public:
    [[nodiscard]] constexpr char operator[](const usize idx) const noexcept
    {
        return m_begin[idx];
    }

    [[nodiscard]] bool operator==(const CString& rhs) noexcept;
    [[nodiscard]] bool operator!=(const CString& rhs) noexcept;

private:
    const char* m_begin;
    usize m_size;
};

namespace cstring_impl {

static constexpr u64 HASH = 0xcbf29ce484222325;
static constexpr u64 PRIME = 0x100000001b3;

[[nodiscard]] constexpr u64 hash_64_fnv1a(const CString& string) noexcept
{
    u64 hash = HASH;
    u64 prime = PRIME;

    for (const auto& c : string) {
        const u8 value = static_cast<u8>(c);
        hash = hash ^ value;
        hash *= prime;
    }

    return hash;
}

} // namespace cstring_impl

} // namespace tundra::core
