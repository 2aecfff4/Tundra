#pragma once
#include "core/std/traits/is_callable.h"
#include "core/std/utils.h"
#include <mutex>

namespace tundra::core {

///
template <typename T, typename LockType, typename MutexType>
class [[nodiscard]] LockGuard {
private:
    T& m_object;
    LockType m_lock;

public:
    LockGuard(T& object, MutexType& mutex)
        : m_object(object)
        , m_lock(mutex)
    {
    }

    ~LockGuard() = default;
    LockGuard(LockGuard&&) = default;
    LockGuard& operator=(LockGuard&&) = default;
    LockGuard(const LockGuard&) = delete;
    LockGuard& operator=(const LockGuard&) = delete;

public:
    operator T&()
    {
        return m_object;
    }

    [[nodiscard]] T* operator->() noexcept
    {
        return &m_object;
    }

    [[nodiscard]] const T* operator->() const noexcept
    {
        return &m_object;
    }

    [[nodiscard]] T& operator*() noexcept
    {
        return m_object;
    }

    [[nodiscard]] const T& operator*() const noexcept
    {
        return m_object;
    }

    LockGuard& operator=(const T& obj)
    {
        m_object = obj;
        return *this;
    }
};

///
template <typename T, typename MutexType = std::mutex>
class Lock {
private:
    mutable T m_object;
    mutable MutexType m_mutex;

public:
    Lock() noexcept
        : m_object()
    {
    }

    Lock(const T& object)
        : m_object(object)
        , m_mutex()
    {
    }

    Lock(T&& object)
        : m_object(core::move(object))
        , m_mutex()
    {
    }

    Lock(const Lock& rhs)
    {
        std::unique_lock lock(rhs.m_mutex);
        m_object = rhs.m_object;
    }

    Lock(Lock&& rhs)
    {
        std::unique_lock lock(rhs.m_mutex);
        m_object = core::move(rhs.m_object);
    }

    Lock& operator=(const T& rhs)
    {
        std::unique_lock lock(m_mutex);
        m_object = rhs;
        return *this;
    }

    Lock& operator=(T&& rhs)
    {
        std::unique_lock lock(m_mutex);
        m_object = core::move(rhs);
        return *this;
    }

    Lock& operator=(const Lock& rhs)
    {
        if (&rhs != this) {
            std::unique_lock lock1(rhs.m_mutex, std::defer_lock);
            std::unique_lock lock2(m_mutex, std::defer_lock);
            std::lock(lock1, lock2);

            m_object = rhs.m_object;
        }
        return *this;
    }

    Lock& operator=(Lock&& rhs) noexcept
    {
        if (&rhs != this) {
            std::unique_lock lock1(rhs.m_mutex, std::defer_lock);
            std::unique_lock lock2(m_mutex, std::defer_lock);
            std::lock(lock1, lock2);

            m_object = core::move(rhs.m_object);
        }
        return *this;
    }

public:
    ///
    void swap(Lock& rhs)
    {
        if (&rhs != this) {
            std::unique_lock lock1(rhs.m_mutex, std::defer_lock);
            std::unique_lock lock2(m_mutex, std::defer_lock);
            std::lock(lock1, lock2);

            core::swap(m_object, rhs.m_object);
        }
    }

    ///
    void swap(T& rhs)
    {
        std::unique_lock lock(m_mutex);
        core::swap(m_object, rhs);
    }

public:
    ///
    [[nodiscard]] LockGuard<T, std::unique_lock<MutexType>, MutexType> lock()
    {
        return LockGuard<T, std::unique_lock<MutexType>, MutexType>(m_object, m_mutex);
    }

    ///
    [[nodiscard]] LockGuard<T, std::unique_lock<MutexType>, MutexType> operator*()
    {
        return this->lock();
    }

public:
    ///
    template <traits::callable<T&> Func>
    [[nodiscard]] auto map(Func&& func)
    {
        using ReturnValue = decltype(std::declval<Func>()(m_object));
        static_assert(
            std::is_reference_v<ReturnValue>, "`func` must return a reference!");
        using M = std::decay_t<ReturnValue>;
        return LockGuard<M, std::unique_lock<MutexType>, MutexType>(
            func(m_object), m_mutex);
    }
};

} // namespace tundra::core
