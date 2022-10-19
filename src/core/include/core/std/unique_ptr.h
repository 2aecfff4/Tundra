#pragma once
#include "core/core.h"
#include "core/std/hash.h"
#include "core/std/utils.h"
#include <concepts>
#include <cstddef>
#include <type_traits>

namespace tundra::core {

namespace unique_ptr_impl {

template <typename T>
class DefaultDeleter {
public:
    constexpr DefaultDeleter() noexcept = default;
    template <typename U>
    constexpr DefaultDeleter(const DefaultDeleter<U>&) noexcept
        requires(std::is_convertible_v<U*, T*>)
    {
    }

public:
    constexpr void operator()(T* const ptr) const noexcept
    {
        // NOLINTNEXTLINE bugprone-sizeof-expression
        static_assert(0u < sizeof(T), "Cannot delete an incomplete type!");
        if constexpr (std::is_array_v<T>) {
            delete[] ptr;
        } else {
            delete ptr;
        }
    }
};

} // namespace unique_ptr_impl

/// Unique pointer implementation.
template <typename T, typename Deleter = unique_ptr_impl::DefaultDeleter<T>>
class UniquePtr : private Deleter {
    static_assert(!std::is_array_v<T>, "#TODO");

    template <typename OtherT, typename OtherDeleter>
    friend class UniquePtr;

private:
    T* m_ptr = nullptr;

public:
    using Type = T;
    using Pointer = Type*;
    using DeleterType = Deleter;

public:
    constexpr UniquePtr() noexcept = default;
    constexpr UniquePtr(std::nullptr_t) noexcept
        : m_ptr(nullptr)
    {
    }

    constexpr explicit UniquePtr(T* ptr) noexcept
        : m_ptr(ptr)
    {
    }

    constexpr explicit UniquePtr(T* ptr, const Deleter& deleter) noexcept
        : Deleter(deleter)
        , m_ptr(ptr)
    {
    }

    UniquePtr(UniquePtr&& rhs) noexcept
        : Deleter(core::move(rhs.get_deleter()))
        , m_ptr(rhs.release())
    {
    }

    template <typename OtherT, typename OtherDeleter>
    UniquePtr(UniquePtr<OtherT, OtherDeleter>&& rhs) //
        noexcept                                     //
        requires(
            !std::is_array_v<OtherT> //
            && std::is_convertible_v<
                   typename UniquePtr<OtherT, OtherDeleter>::Pointer,
                   Pointer>                                     //
            && std::is_constructible_v<Deleter, OtherDeleter&&> //
            && (!std::is_same_v<T, OtherT> || !std::is_same_v<Deleter, OtherDeleter>))
        : Deleter(core::move(rhs.get_deleter()))
        , m_ptr(rhs.release())
    {
    }

    UniquePtr& operator=(UniquePtr&& rhs) noexcept
    {
        if (&rhs != this) {
            this->reset(rhs.release());
            this->get_deleter() = core::move(rhs.get_deleter());
        }
        return *this;
    }

    template <typename OtherT, typename OtherDeleter>
    UniquePtr& operator=(UniquePtr<OtherT, OtherDeleter>&& rhs) //
        noexcept                                                //
        requires(
            !std::is_array_v<OtherT> //
            && std::is_convertible_v<
                   typename UniquePtr<OtherT, OtherDeleter>::Pointer,
                   Pointer>                                  //
            && std::is_assignable_v<Deleter, OtherDeleter&&> //
            && (!std::is_same_v<T, OtherT> || !std::is_same_v<Deleter, OtherDeleter>))
    {
        this->reset(rhs.release());
        this->get_deleter() = core::move(rhs.get_deleter());
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) noexcept
    {
        this->reset();
        return *this;
    }

    ~UniquePtr() noexcept
    {
        this->reset();
    }

    UniquePtr(const UniquePtr&) noexcept = delete;
    UniquePtr& operator=(const UniquePtr&) noexcept = delete;

public:
    [[nodiscard]] constexpr bool operator==(const UniquePtr&) const noexcept = default;
    [[nodiscard]] constexpr bool operator!=(const UniquePtr&) const noexcept = default;

public:
    /// Returns true if object is valid, otherwise false.
    [[nodiscard]] constexpr bool is_valid() const noexcept
    {
        return m_ptr != nullptr;
    }

    /// Returns pointer to managed object.
    [[nodiscard]] constexpr T* get() const noexcept
    {
        return m_ptr;
    }

    /// Replaces the managed object.
    /// If the passed in pointer is the same as the owned pointer, nothing is done.
    ///
    /// # Parameters
    /// `ptr` - New pointer to take ownership.
    inline void reset(T* const ptr = nullptr) noexcept
    {
        if (ptr != m_ptr) {
            T* const old { m_ptr };
            m_ptr = ptr;

            if (old != nullptr) {
                this->get_deleter()(old);
            }
        }
    }

    /// Returns a pointer to the managed object and releases the ownership.
    /// It's user responsibility to delete the pointer afterwards.
    ///
    /// # Example
    /// UniquePtr<char> u = make_unique<char>();
    /// char* ptr = u.release();
    /// delete ptr;
    [[nodiscard]] constexpr T* release() noexcept
    {
        T* const temp { m_ptr };
        m_ptr = nullptr;
        return temp;
    }

public:
    [[nodiscard]] constexpr decltype(auto) operator*() const noexcept
    {
        return *m_ptr;
    }

    [[nodiscard]] constexpr T* operator->() const noexcept
    {
        return this->get();
    }

    [[nodiscard]] constexpr explicit operator bool() const noexcept
    {
        return this->is_valid();
    }

    [[nodiscard]] constexpr bool operator!() const noexcept
    {
        return !this->is_valid();
    }

private:
    [[nodiscard]] constexpr Deleter& get_deleter() noexcept
    {
        return static_cast<Deleter&>(*this);
    }

    [[nodiscard]] constexpr const Deleter& get_deleter() const noexcept
    {
        return static_cast<const Deleter&>(*this);
    }
};

template <typename L, typename LAllocator, typename R, typename RAllocator>
[[nodiscard]] constexpr bool operator==(
    const UniquePtr<L, LAllocator>& lhs, const UniquePtr<R, RAllocator>& rhs) //
    noexcept                                                                  //
    requires(std::equality_comparable_with<
             typename UniquePtr<L, LAllocator>::Pointer,
             typename UniquePtr<R, RAllocator>::Pointer>)
{
    return lhs.get() == rhs.get();
}

template <typename L, typename LAllocator, typename R, typename RAllocator>
[[nodiscard]] constexpr bool operator!=(
    const UniquePtr<L, LAllocator>& lhs, const UniquePtr<R, RAllocator>& rhs) //
    noexcept                                                                  //
    requires(std::equality_comparable_with<
             typename UniquePtr<L, LAllocator>::Pointer,
             typename UniquePtr<R, RAllocator>::Pointer>)
{
    return lhs.get() != rhs.get();
}

template <typename T>
[[nodiscard]] constexpr bool operator==(std::nullptr_t, const UniquePtr<T>& rhs) noexcept
{
    return !rhs.is_valid();
}

template <typename T>
[[nodiscard]] constexpr bool operator==(const UniquePtr<T>& lhs, std::nullptr_t) noexcept
{
    return !lhs.is_valid();
}

template <typename T>
[[nodiscard]] constexpr bool operator!=(const UniquePtr<T>& lhs, std::nullptr_t) noexcept
{
    return lhs.is_valid();
}

template <typename T, typename R>
[[nodiscard]] constexpr bool operator==(
    const UniquePtr<T>& lhs, const R* const ptr) noexcept
{
    return lhs.get() == ptr;
}

template <typename T, typename R>
[[nodiscard]] constexpr bool operator!=(
    const UniquePtr<T>& lhs, const R* const ptr) noexcept
{
    return lhs.get() != ptr;
}

/// Disabled for array types
template <typename T, typename... Args>
[[nodiscard]] constexpr UniquePtr<T> make_unique(Args&&... args) //
    requires(!std::is_array_v<T>)
{
    return UniquePtr<T> { new T(static_cast<Args&&>(args)...) };
}

template <typename T>
[[nodiscard]] constexpr UniquePtr<T> make_unique(const usize size) //
    requires(std::is_array_v<T> && (std::extent_v<T> == 0)) = delete;

template <typename T, typename... Args>
[[nodiscard]] constexpr UniquePtr<T> make_unique(Args&&...) //
    requires(std::is_array_v<T> && (std::extent_v<T> != 0)) = delete;

template <typename T, typename Deleter>
struct Hash<UniquePtr<T, Deleter>> {
    [[nodiscard]] usize operator()(const UniquePtr<T, Deleter>& v) const noexcept
    {
        return Hash<T> {}(v.get());
    }
};

} // namespace tundra::core
