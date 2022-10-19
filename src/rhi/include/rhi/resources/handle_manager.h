#pragma once
#include "rhi/rhi_export.h"
#include "core/core.h"
#include "core/std/assert.h"
#include "core/std/containers/array.h"
#include "core/std/containers/deque.h"
#include "core/std/expected.h"
#include "core/std/sync/rw_lock.h"
#include "core/std/traits/is_callable.h"
#include "core/std/unique_ptr.h"
#include "core/std/utils.h"

#if TNDR_BUILD_DEBUG
#include "core/logger.h"
#endif

namespace tundra::rhi {

///
enum class HandleManagerError : u8 {
    NullHandle,
    InvalidHandle,
};

/// `HandleManager` is thread safe.
template <typename FrontendType, typename BackendType>
class HandleManager {
private:
    ///
    struct ResourceStorage {
        core::UniquePtr<BackendType> object;
        u32 generation = 0;
    };

    ///
    struct Inner {
        core::Array<ResourceStorage> resources;
        core::Deque<FrontendType> free_list;
    };

    ///
    static constexpr usize MIN_NUM_FREE_HANDLES = 1024;

private:
    mutable core::RwLock<Inner> m_inner;
    const char* m_name;

public:
    explicit HandleManager(const char* name) noexcept
        : m_name(name)
    {
    }

    ~HandleManager() noexcept
    {
#if TNDR_BUILD_DEBUG
        auto inner = m_inner.read();

        usize count = 0;
        for (const ResourceStorage& resource_storage : inner->resources) {
            count += static_cast<usize>(resource_storage.object.is_valid());
        }

        if (count > 0) {
            tndr_warn("Leaked {} `{}` resources.", count, m_name);
        }
#endif
    }

public:
    template <typename... Args>
    [[nodiscard]] FrontendType add(Args&&... args) noexcept
    {
        auto object = core::make_unique<BackendType>(core::forward<Args>(args)...);

        auto inner = m_inner.write();

        if (inner->free_list.size() < MIN_NUM_FREE_HANDLES) {
            const usize index = inner->resources.size();

            inner->resources.push_back(HandleManager::ResourceStorage {
                .object = core::move(object),
            });

            return FrontendType(static_cast<u64>(index), 0);
        } else {
            const FrontendType old_handle = inner->free_list.front();
            inner->free_list.pop_front();

            const FrontendType new_handle(
                old_handle.get_index(), old_handle.get_generation() + 1);

            inner->resources[new_handle.get_index()].object = core::move(object);

            return new_handle;
        }
    }

    /// Returns true if an object was alive, otherwise false.
    [[nodiscard]] bool destroy(const FrontendType handle) noexcept
    {
        auto inner = m_inner.write();

        const u64 index = handle.get_index();
        tndr_assert(index < inner->resources.size(), "");

        HandleManager::ResourceStorage& resource_storage = inner->resources[index];
        const bool is_valid = (resource_storage.generation == handle.get_generation()) &&
                              resource_storage.object.is_valid();

        if (is_valid) {
            resource_storage.object.reset();
            resource_storage.generation += 1;

            if ((handle.get_generation() + 1) < (FrontendType::MAX_GENERATION - 1)) {
                inner->free_list.push_back(handle);
            }

            return true;
        } else {
            return false;
        }
    }

    /// Returns true if object is alive, otherwise false.
    [[nodiscard]] bool is_valid(const FrontendType handle) const noexcept
    {
        auto inner = m_inner.read();

        const u64 index = handle.get_index();
        tndr_assert(index < inner->resources.size(), "");

        const HandleManager::ResourceStorage& resource_storage = inner->resources[index];
        const bool is_valid = (resource_storage.generation == handle.get_generation()) &&
                              resource_storage.object.is_valid();

        return is_valid;
    }

    [[nodiscard]] const char* get_name() const noexcept
    {
        return m_name;
    }

    /// #TODO: Maybe come up with the better name?
    /// `HandleManager` does not return references to `BackendType`, to ensure that thread safety is not violated.
    /// Closure `func` is only executed when a handle is valid.
    ///
    /// ```
    /// handle_manager.with(handle, [](const Buffer& buffer) {
    ///     // Do something.
    /// });
    /// ```
    template <
        core::traits::callable<const BackendType&> Func,
        typename Data = std::invoke_result_t<Func, const BackendType&>>
    [[nodiscard]] core::Expected<Data, HandleManagerError> with(
        const FrontendType handle, Func func) const noexcept
    {
        if (handle.is_null()) {
            return core::make_unexpected(HandleManagerError::NullHandle);
        }

        auto inner = m_inner.read();

        const u64 index = handle.get_index();
        if (index >= inner->resources.size()) {
            return core::make_unexpected(HandleManagerError::InvalidHandle);
        }

        const HandleManager::ResourceStorage& resource_storage = inner->resources[index];
        const bool is_valid = (resource_storage.generation == handle.get_generation()) &&
                              resource_storage.object.is_valid();

        if (is_valid) {
            if constexpr (std::is_same_v<Data, void>) {
                func(static_cast<const BackendType&>(*resource_storage.object));
                return {};
            } else {
                return func(static_cast<const BackendType&>(*resource_storage.object));
            }
        } else {
            return core::make_unexpected(HandleManagerError::InvalidHandle);
        }
    }

    /// @see `HandleManager::with`
    template <
        core::traits::callable<BackendType&> Func,
        typename Data = std::invoke_result_t<Func, BackendType&>>
    [[nodiscard]] core::Expected<Data, HandleManagerError> with_mut(
        const FrontendType handle, Func func) noexcept
    {
        if (handle.is_null()) {
            return core::make_unexpected(HandleManagerError::NullHandle);
        }

        auto inner = m_inner.write();

        const u64 index = handle.get_index();
        if (index >= inner->resources.size()) {
            return core::make_unexpected(HandleManagerError::InvalidHandle);
        }

        HandleManager::ResourceStorage& resource_storage = inner->resources[index];
        const bool is_valid = (resource_storage.generation == handle.get_generation()) &&
                              resource_storage.object.is_valid();

        if (is_valid) {
            if constexpr (std::is_same_v<Data, void>) {
                func(*resource_storage.object);
                return {};
            } else {
                return func(*resource_storage.object);
            }
        } else {
            return core::make_unexpected(HandleManagerError::InvalidHandle);
        }
    }
};

} // namespace tundra::rhi
