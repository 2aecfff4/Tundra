#pragma once
#include "rhi/rhi_export.h"
#include "core/core.h"
#include "core/std/containers/hash_map.h"
#include "core/std/containers/hash_set.h"
#include "core/std/function.h"
#include "core/std/sync/rw_lock.h"
#include <atomic>

namespace tundra::rhi {

/// # Background
/// In modern low-level APIs we are responsible for managing resources lifetimes,
/// not the driver.
/// What does it mean for us?
/// For example, while a resource is used by a command buffer, we can't destroy it.
/// We have to wait until execution of the command buffer finishes
/// (we can use semaphores/fences for that), and only then we can safely destroy the resource.
///
/// # The goal
/// The role of the resource tracker is to track resources lifetimes by using
/// an atomic reference counter, and destroying them only when they are not referenced.
///
/// # Thread safety
/// `ResourceTracker` is thread safe.
/// `Resources` is **not** thread safe, and should be used on per-thread basis.
class RHI_API ResourceTracker {
public:
    /// `Resources` is **not** thread safe, and should be used on per-thread basis.
    struct RHI_API Resources {
        core::HashSet<u64> resources;

        void add_reference(ResourceTracker& resource_tracker, const u64 resource) noexcept;
    };

private:
    ///
    struct Resource {
        core::Function<void()> resource_destructor;
        std::atomic<i32> ref_count = 1;

        Resource(core::Function<void()>&& func) noexcept
            : resource_destructor(core::move(func))
        {
        }

        Resource(const Resource& rhs) noexcept
        {
            resource_destructor = rhs.resource_destructor;
            ref_count.store(
                rhs.ref_count.load(std::memory_order_acquire), std::memory_order_release);
        }

        Resource& operator=(const Resource& rhs) noexcept
        {
            if (&rhs != this) {
                resource_destructor = rhs.resource_destructor;
                ref_count.store(
                    rhs.ref_count.load(std::memory_order_acquire),
                    std::memory_order_release);
            }
            return *this;
        }
    };

    ///
    struct Inner {
        core::HashMap<u64, Resource> resources;
    };

private:
    core::RwLock<Inner> m_inner;

public:
    void add_resource(const u64 resource, core::Function<void()>&& destructor) noexcept;
    void add_reference(const u64 resource) noexcept;
    void remove_reference(const u64 resource) noexcept;
    void remove_references(Resources& resources) noexcept;
};

} // namespace tundra::rhi
