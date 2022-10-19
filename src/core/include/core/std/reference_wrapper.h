#pragma once
#include "core/std/utils.h"
#include <type_traits>

namespace tundra::core {

///
template <typename T>
class ReferenceWrapper {
public:
    using Type = T;

    template <typename U>
    constexpr ReferenceWrapper(U&& u) noexcept //
        requires(std::conjunction_v<
                 std::negation<std::is_same<std::remove_cvref_t<U>, ReferenceWrapper>>,
                 std::is_lvalue_reference<U>>)
    {
        T& temp = static_cast<U&&>(u);
        m_ptr = core::addressof(temp);
    }
    ReferenceWrapper(const ReferenceWrapper&) noexcept = default;
    ReferenceWrapper& operator=(const ReferenceWrapper& x) noexcept = default;

    // access
    [[nodiscard]] constexpr operator T&() const noexcept
    {
        return *m_ptr;
    }
    [[nodiscard]] constexpr T& get() const noexcept
    {
        return *m_ptr;
    }

private:
    T* m_ptr;
};

// deduction guides
template <typename T>
ReferenceWrapper(T&) -> ReferenceWrapper<T>;

} // namespace tundra::core
