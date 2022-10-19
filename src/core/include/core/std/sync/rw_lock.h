#pragma once
#include "core/std/utils.h"
#include <mutex>
#include <shared_mutex>

namespace tundra::core {

///
template <typename T, typename LockType>
class [[nodiscard]] RwLockGuard {
private:
    T& m_object;
    LockType m_lock;

public:
    RwLockGuard(T& object, std::shared_mutex& mutex)
        : m_object(object)
        , m_lock(mutex)
    {
    }

    ~RwLockGuard() = default;
    RwLockGuard(RwLockGuard&&) = default;
    RwLockGuard& operator=(RwLockGuard&&) = default;
    RwLockGuard(const RwLockGuard&) = delete;
    RwLockGuard& operator=(const RwLockGuard&) = delete;

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

    RwLockGuard& operator=(const T& obj)
    {
        m_object = obj;
        return *this;
    }
};

///
template <typename T>
class RwLock {
public:
    using RwReadLockGuard = RwLockGuard<T, std::shared_lock<std::shared_mutex>>;
    using RwWriteLockGuard = RwLockGuard<T, std::unique_lock<std::shared_mutex>>;

private:
    mutable T m_object;
    mutable std::shared_mutex m_mutex;

public:
    RwLock() noexcept = default;
    RwLock(const T& object)
        : m_object(object)
        , m_mutex()
    {
    }

    RwLock(T&& object)
        : m_object(core::move(object))
        , m_mutex()
    {
    }

    RwLock(const RwLock& rhs)
    {
        std::unique_lock lock(rhs.m_mutex);
        m_object = rhs.m_object;
    }

    RwLock(RwLock&& rhs)
    {
        std::unique_lock lock(rhs.m_mutex);
        m_object = core::move(rhs.m_object);
    }

    RwLock& operator=(const T& rhs)
    {
        std::unique_lock lock(m_mutex);
        m_object = rhs;
        return *this;
    }

    RwLock& operator=(T&& rhs)
    {
        std::unique_lock lock(m_mutex);
        m_object = core::move(rhs);
        return *this;
    }

    RwLock& operator=(const RwLock& rhs)
    {
        if (&rhs != this) {
            std::unique_lock lock1(rhs.m_mutex, std::defer_lock);
            std::unique_lock lock2(m_mutex, std::defer_lock);
            std::lock(lock1, lock2);

            m_object = rhs.m_object;
        }
        return *this;
    }

    RwLock& operator=(RwLock&& rhs) noexcept
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
    void swap(RwLock& rhs)
    {
        if (&rhs != this) {
            std::unique_lock lock1(rhs.m_mutex, std::defer_lock);
            std::unique_lock lock2(m_mutex, std::defer_lock);
            std::lock(lock1, lock2);

            core::swap(m_object, rhs.m_object);
        }
    }

    void swap(T& rhs)
    {
        std::unique_lock lock(m_mutex);
        core::swap(m_object, rhs);
    }

public:
    [[nodiscard]] RwReadLockGuard read()
    {
        return RwReadLockGuard(m_object, m_mutex);
    }

    [[nodiscard]] RwWriteLockGuard write()
    {
        return RwWriteLockGuard(m_object, m_mutex);
    }

    [[nodiscard]] RwWriteLockGuard operator*()
    {
        return this->write();
    }
};

} // namespace tundra::core
