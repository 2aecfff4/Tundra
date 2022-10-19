#include "rhi/resources/resource_tracker.h"
#include "core/std/panic.h"

namespace tundra::rhi {

/////////////////////////////////////////////////////////////////////////////////////////
// ResourceTracker::Resources

void ResourceTracker::Resources::add_reference(
    ResourceTracker& resource_tracker, const u64 resource) noexcept
{
    if (auto it = resources.find(resource); it == resources.end()) {
        resources.insert(resource);
        resource_tracker.add_reference(resource);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
// ResourceTracker

void ResourceTracker::add_resource(
    const u64 resource, core::Function<void()>&& destructor) noexcept
{
    auto inner = m_inner.write();

    inner->resources.insert({
        resource,
        ResourceTracker::Resource {
            core::move(destructor),
        },
    });
}

void ResourceTracker::add_reference(const u64 resource) noexcept
{
    auto inner = m_inner.read();

    if (auto it = inner->resources.find(resource); it != inner->resources.end()) {
        (*it).second.ref_count.fetch_add(1, std::memory_order::relaxed);
    } else {
        core::panic("Resource `{}` does not exist.", resource);
    }
}

void ResourceTracker::remove_reference(const u64 resource) noexcept
{
    const i32 value = [&] {
        auto inner = m_inner.read();

        if (auto it = inner->resources.find(resource); it != inner->resources.end()) {
            return (*it).second.ref_count.fetch_sub(1, std::memory_order::relaxed);
        } else {
            core::panic("Resource `{}` does not exist.", resource);
        }
    }();

    if ((value - 1) <= 0) {
        auto inner = m_inner.write();

        const auto it = inner->resources.find(resource);
        (*it).second.resource_destructor();
        inner->resources.erase(it);
    }
}

void ResourceTracker::remove_references(ResourceTracker::Resources& resources) noexcept
{
    for (const u64 resource : resources.resources) {
        this->remove_reference(resource);
    }
}

} // namespace tundra::rhi
