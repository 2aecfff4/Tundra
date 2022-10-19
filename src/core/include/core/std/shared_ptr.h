#pragma once
#include "core/std/utils.h"
#include <memory>

namespace tundra::core {

///
template <typename T>
using SharedPtr = std::shared_ptr<T>;

///
template <typename T>
using WeakPtr = std::weak_ptr<T>;

///
template <typename T>
class EnableSharedFromThis : public std::enable_shared_from_this<T> {
};

///
template <typename T, typename... Args>
[[nodiscard]] core::SharedPtr<T> make_shared(Args&&... args)
{
    return std::make_shared<T>(core::forward<Args>(args)...);
}

} // namespace tundra::core
