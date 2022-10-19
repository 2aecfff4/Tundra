#pragma once
#include "core/core.h"
#include "core/std/assert.h"
#include "core/std/traits/declval.h"
#include <type_traits>

namespace tundra::core {

namespace span_impl {

///
template <typename T>
concept array_container = requires(T& c)
{
    { c.data() };
    { c.size() };
    requires std::is_pointer_v<decltype(c.data())>;
};

} // namespace span_impl

///
template <typename T>
class Span {
private:
    T* m_data;
    usize m_size;

private:
    template <typename T2>
    friend class Span;

public:
    constexpr Span() noexcept
        : m_data(nullptr)
        , m_size(0)
    {
    }

    constexpr Span(T& value) noexcept
        : m_data(&value)
        , m_size(1)
    {
    }

    constexpr Span(T* data, const usize size) noexcept
        : m_data(data)
        , m_size(size)
    {
    }

    constexpr Span(T* first, T* last) noexcept
        : m_data(first)
        , m_size(static_cast<usize>(last - first))
    {
    }

    template <usize extent>
    constexpr explicit Span(T (&array)[extent]) noexcept
        : m_data(array)
        , m_size(extent)
    {
    }

    template <typename C>
    constexpr Span(C& c) noexcept //
        requires(
            span_impl::array_container<C>&& std::is_convertible_v<
                std::remove_pointer_t<decltype(c.data())> (*)[],
                T (*)[]> &&
            !std::is_const_v<T>)
        : m_data(c.data())
        , m_size(c.size())
    {
    }

    template <typename C>
    constexpr Span(const C& c) noexcept //
        requires(span_impl::array_container<C>&& std::is_convertible_v<
                 std::remove_pointer_t<decltype(c.data())> (*)[],
                 T (*)[]>&& std::is_const_v<T>)
        : m_data(c.data())
        , m_size(c.size())
    {
    }

    constexpr Span(const Span&) noexcept = default;
    constexpr Span& operator=(const Span&) noexcept = default;

    constexpr Span(Span&& rhs) noexcept
        : m_data(rhs.m_data)
        , m_size(rhs.m_size)
    {
        rhs.m_data = nullptr;
        rhs.m_size = 0;
    }

    constexpr Span& operator=(Span&& rhs) noexcept
    {
        if (&rhs != this) {
            m_data = rhs.m_data;
            rhs.m_data = nullptr;

            m_size = rhs.m_size;
            rhs.m_size = 0;
        }
        return *this;
    }

    template <typename T2>
    constexpr Span(const Span<T2>& rhs) noexcept //
        requires(std::is_convertible_v<T2, T>)   //
        : m_data(rhs.m_data)
        , m_size(rhs.m_size)
    {
    }

    template <typename T2>
    constexpr Span& operator=(const Span<T2>& rhs) noexcept //
        requires(std::is_convertible_v<T2, T>)              //
    {
        if (&rhs != this) {
            m_data = rhs.m_data;
            m_size = rhs.m_size;
        }
        return *this;
    }

    template <typename T2>
    constexpr Span(Span<T2>&& rhs) noexcept    //
        requires(std::is_convertible_v<T2, T>) //
        : m_data(rhs.m_data)
        , m_size(rhs.m_size)
    {
        rhs.m_data = nullptr;
        rhs.m_size = 0;
    }

    template <typename T2>
    constexpr Span& operator=(Span<T2>&& rhs) noexcept //
        requires(std::is_convertible_v<T2, T>)         //
    {
        if (&rhs != this) {
            m_data = rhs.m_data;
            rhs.m_data = nullptr;

            m_size = rhs.m_size;
            rhs.m_size = 0;
        }
        return *this;
    }

public:
    [[nodiscard]] constexpr T& operator[](const usize idx) noexcept
    {
        tndr_debug_assert(idx < m_size, "");
        return m_data[idx];
    }

    [[nodiscard]] constexpr const T& operator[](const usize idx) const noexcept
    {
        tndr_debug_assert(idx < m_size, "");
        return m_data[idx];
    }

public:
    [[nodiscard]] constexpr T* begin() noexcept
    {
        return m_data;
    }

    [[nodiscard]] constexpr const T* begin() const noexcept
    {
        return m_data;
    }

    [[nodiscard]] constexpr T* end() noexcept
    {
        return m_data + m_size;
    }

    [[nodiscard]] constexpr const T* end() const noexcept
    {
        return m_data + m_size;
    }

    [[nodiscard]] constexpr T* data() noexcept
    {
        return m_data;
    }

    [[nodiscard]] constexpr const T* data() const noexcept
    {
        return m_data;
    }

    [[nodiscard]] constexpr usize size() const noexcept
    {
        return m_size;
    }

    [[nodiscard]] constexpr bool is_empty() const noexcept
    {
        return this->size() == 0;
    }
};

// Deduction Guides

template <typename T>
Span(T&) -> Span<T>;

template <typename T, usize extent>
Span(T (&)[extent]) -> Span<T>;

template <
    span_impl::array_container Container,
    typename T = std::remove_pointer_t<decltype(traits::declval<Container&>().data())>>
Span(Container&) -> Span<T>;

template <
    span_impl::array_container Container,
    typename T =
        std::remove_pointer_t<decltype(traits::declval<const Container&>().data())>>
Span(const Container&) -> Span<T>;

///
template <typename T>
[[nodiscard]] constexpr Span<T> as_span(T& obj) noexcept
{
    return Span<T>(obj);
}

///
template <typename T, usize extent>
[[nodiscard]] constexpr Span<T> as_span(T (&arr)[extent]) noexcept
{
    return Span<T>(arr);
}

///
template <
    span_impl::array_container Container,
    typename T = std::remove_pointer_t<decltype(traits::declval<Container&>().data())>>
[[nodiscard]] constexpr Span<T> as_span(Container& c) noexcept
{
    return Span<T>(c);
}

///
template <
    span_impl::array_container Container,
    typename T =
        std::remove_pointer_t<decltype(traits::declval<const Container&>().data())>>
[[nodiscard]] constexpr Span<T> as_span(const Container& c) noexcept
{
    return Span<T>(c);
}

///
template <
    span_impl::array_container Container,
    typename T = std::remove_pointer_t<decltype(traits::declval<Container&&>().data())>>
[[nodiscard]] constexpr Span<T> as_span(Container&& c) noexcept = delete;

///
template <typename T, typename R = std::conditional_t<std::is_const_v<T>, const char, char>>
[[nodiscard]] constexpr Span<R> as_byte_span(T& obj) noexcept
{
    return Span<R> { reinterpret_cast<R*>(&obj), sizeof(T) };
}

///
template <
    span_impl::array_container Container,
    typename R = std::conditional_t<std::is_const_v<Container>, const char, char>>
[[nodiscard]] constexpr Span<R> as_byte_span(Container& c) noexcept
{
    return Span<R> { reinterpret_cast<R*>(c.data()), c.size() * sizeof(*c.data()) };
}

///
template <typename T, typename R = std::conditional_t<std::is_const_v<T>, const char, char>>
[[nodiscard]] constexpr Span<R> as_byte_span(T&& obj) noexcept = delete;

} // namespace tundra::core
