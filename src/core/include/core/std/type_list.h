#pragma once
#include "core/core.h"
#include <type_traits>

namespace tundra::core {

template <typename... Ts>
struct TypeList {
    using Type = TypeList;

    /// Returns number of types.
    [[nodiscard]] static constexpr usize count() noexcept
    {
        return sizeof...(Ts);
    }
};

namespace type_at_private {

template <usize idx, typename List>
struct TypeAtImpl;

template <typename T, typename... Ts>
struct TypeAtImpl<0, TypeList<T, Ts...>> {
    using Type = T;
};

template <usize idx, typename T, typename... Ts>
struct TypeAtImpl<idx, TypeList<T, Ts...>> {
    using Type = typename TypeAtImpl<idx - 1, TypeList<Ts...>>::Type;
};

} // namespace type_at_private

template <usize idx, typename List>
struct TypeAt;

template <usize idx, typename... Ts>
struct TypeAt<idx, TypeList<Ts...>> {
    static_assert(sizeof...(Ts) != 0, "`TypeList` is empty!");
    static_assert(sizeof...(Ts) > idx, "Index out of bounds!");
    using Type = typename type_at_private::TypeAtImpl<idx, TypeList<Ts...>>::Type;
};

template <usize idx, typename... Ts>
using TypeAtT = typename TypeAt<idx, Ts...>::Type;

namespace index_of_private {

template <typename>
[[nodiscard]] constexpr usize get_type_index([[maybe_unused]] usize index) noexcept
{
    return static_cast<usize>(-1);
}

template <typename IndexedType, typename T, typename... Ts>
[[nodiscard]] constexpr usize get_type_index(usize ind = 0) noexcept
{
    if constexpr (std::is_same_v<IndexedType, T>) {
        return ind;
    } else {
        return get_type_index<IndexedType, Ts...>(ind + 1);
    }
}

} // namespace index_of_private

template <typename T, typename List>
struct TypeIndexOf;

template <typename T, typename... Types>
struct TypeIndexOf<T, TypeList<Types...>>
    : public std::
          integral_constant<std::size_t, index_of_private::get_type_index<T, Types...>()> {
};

namespace unique_type_list_private {

template <typename T, typename... Ts>
struct UniqueTypeListImpl {
    using type = T;
};

template <typename... Ts, typename U, typename... Us>
struct UniqueTypeListImpl<TypeList<Ts...>, U, Us...>
    : public std::conditional_t<
          (std::is_same_v<U, Ts> || ...),
          UniqueTypeListImpl<TypeList<Ts...>, Us...>,
          UniqueTypeListImpl<TypeList<Ts..., U>, Us...>> {
};

} // namespace unique_type_list_private

template <typename... Ts>
using UniqueTypeList =
    typename unique_type_list_private::UniqueTypeListImpl<TypeList<>, Ts...>::type;

namespace rename_private {

template <typename A, template <typename...> typename B>
struct RenameImpl;

template <template <typename...> typename A, typename... T, template <typename...> typename B>
struct RenameImpl<A<T...>, B> {
    using Type = B<T...>;
};

} // namespace rename_private

template <typename A, template <typename...> typename B>
using Rename = typename rename_private::RenameImpl<A, B>::Type;

namespace overload_resolution_private {

template <typename T>
struct Overload {
    using Fn = T (*)(T);
    operator Fn() const noexcept;
};

template <typename... Ts>
struct OverloadSetImpl;

template <typename... Ts>
struct OverloadSetImpl<TypeList<Ts...>> : public Overload<Ts>... {
};

} // namespace overload_resolution_private

template <typename... Ts>
struct OverloadSet
    : public overload_resolution_private::OverloadSetImpl<UniqueTypeList<Ts...>> {
};

template <
    typename T,
    typename OverloadSet,
    typename Result = decltype(std::declval<OverloadSet>()(std::declval<T>()))>
struct OverloadResolution {
    using Type = Result;
};

template <typename T, typename OverloadSet>
using OverloadResolutionT = typename OverloadResolution<T, OverloadSet>::Type;

} // namespace tundra::core
