#pragma once
#include "core/core.h"
#include "core/std/hash.h"
#include "core/std/type_list.h"
#include "core/std/utils.h"
#include <type_traits>

namespace tundra::core {

namespace concepts {

template <typename T>
concept has_tuple_size = requires
{
    T::TUPLE_SIZE;
};

} // namespace concepts

namespace tuple_impl {

template <usize N, typename T>
class TupleElement {
public:
    T element;

    using Type = std::decay_t<T>;

public:
    constexpr TupleElement() noexcept = default;

    template <typename U>
    constexpr TupleElement(U&& value)                   //
        noexcept(std::is_nothrow_constructible_v<T, U>) //
        requires(
            std::is_constructible_v<T, U> &&
            !std::is_same_v<std::decay_t<U>, TupleElement>) //
        : element(core::forward<U>(value))
    {
    }

    template <usize OtherN, typename OtherT>
    constexpr TupleElement(TupleElement<OtherN, OtherT>&& rhs) //
        noexcept(std::is_nothrow_constructible_v<T, OtherT&&>) //
        requires(std::is_constructible_v<T, OtherT&&>)         //
        : element(core::forward<OtherT>(rhs.element))
    {
    }

    template <usize OtherN, typename OtherT>
    constexpr TupleElement& operator=(TupleElement<OtherN, OtherT>&& rhs) //
        noexcept(std::is_nothrow_assignable_v<T, OtherT&&>)               //
        requires(std::is_assignable_v<T, OtherT&&>)                       //
    {
        if (&rhs != this) {
            element = core::forward<OtherT>(rhs.element);
        }
        return *this;
    }

    template <usize OtherN, typename OtherT>
    constexpr TupleElement(const TupleElement<OtherN, OtherT>& rhs) //
        noexcept(std::is_nothrow_constructible_v<T, const OtherT&>) //
        requires(std::is_constructible_v<T, const OtherT&>)         //
        : element(rhs.element)
    {
    }

    template <usize OtherN, typename OtherT>
    constexpr TupleElement& operator=(const TupleElement<OtherN, OtherT>& rhs) //
        noexcept(std::is_nothrow_assignable_v<T, const OtherT&>)               //
        requires(std::is_constructible_v<T, const OtherT&>)                    //
    {
        if (&rhs != this) {
            element = rhs.element;
        }
        return *this;
    }

    constexpr ~TupleElement() noexcept(std::is_nothrow_destructible_v<T>) = default;

public:
    [[nodiscard]] constexpr T& get() noexcept
    {
        return element;
    }

    [[nodiscard]] constexpr const T& get() const noexcept
    {
        return element;
    }

public:
    template <typename Other>
    [[nodiscard]] constexpr bool operator==(
        const TupleElement<N, Other>& rhs) const noexcept
    {
        return element == rhs.element;
    }

    template <typename Other>
    [[nodiscard]] constexpr bool operator!=(
        const TupleElement<N, Other>& rhs) const noexcept
    {
        return element != rhs.element;
    }

    template <typename Other>
    [[nodiscard]] constexpr bool operator<(
        const TupleElement<N, Other>& rhs) const noexcept
    {
        return element < rhs.element;
    }

    template <typename Other>
    [[nodiscard]] constexpr bool operator<=(
        const TupleElement<N, Other>& rhs) const noexcept
    {
        return element <= rhs.element;
    }

    template <typename Other>
    [[nodiscard]] constexpr bool operator>(
        const TupleElement<N, Other>& rhs) const noexcept
    {
        return element < rhs.element;
    }

    template <typename Other>
    [[nodiscard]] constexpr bool operator>=(
        const TupleElement<N, Other>& rhs) const noexcept
    {
        return element <= rhs.element;
    }
};

template <typename N, typename... Ts>
class TupleImpl;

/// Non-recursive tuple implementation.
template <usize... N, typename... Ts>
class TupleImpl<std::integer_sequence<usize, N...>, Ts...>
    : public TupleElement<N, Ts>... {
private:
    using Types = TypeList<Ts...>;

public:
    template <usize I>
    using Type = TypeAtT<I, Types>;

public:
    static constexpr bool HAS_DUPLICATES =
        !std::is_same_v<core::TypeList<Ts...>, core::UniqueTypeList<Ts...>>;
    static constexpr usize TUPLE_SIZE = sizeof...(Ts);

public:
    constexpr TupleImpl() noexcept = default;
    template <typename... Args>
    constexpr TupleImpl(Args&&... args) //
        requires(
            (!(std::is_same_v<std::decay_t<Args>, TupleImpl> && ...)) //
            && (sizeof...(Args) == sizeof...(Ts))                     //
            && (std::is_constructible_v<Ts, Args> && ...)             //
            )
        : TupleElement<N, Ts>(core::forward<Args>(args))...
    {
    }

    constexpr TupleImpl(TupleImpl&&) noexcept = default;

    template <usize... R, typename... Types>
    constexpr TupleImpl(TupleImpl<std::integer_sequence<usize, R...>, Types...>&& rhs) //
        noexcept(std::conjunction_v<std::is_nothrow_assignable<Ts, Types&&>...>)       //
        requires(                                                                      //
            (sizeof...(Ts) == sizeof...(Types))                                        //
            && (std::is_constructible_v<Ts, Types&&> && ...)                           //
            && !(std::is_same_v<Ts, Types> && ...))                                    //
        : TupleElement<N, Ts>(core::forward<Types>(rhs.template get<R>()))...
    {
    }

    constexpr TupleImpl& operator=(TupleImpl&&) noexcept = default;

    template <usize... R, typename... Types>
    constexpr TupleImpl& operator=(
        TupleImpl<std::integer_sequence<usize, R...>, Types...>&& rhs)           //
        noexcept(std::conjunction_v<std::is_nothrow_assignable<Ts, Types&&>...>) //
        requires(                                                                //
            (sizeof...(Ts) == sizeof...(Types))                                  //
            && (std::is_assignable_v<Ts, Types&&> && ...)                        //
            && !(std::is_same_v<Ts, Types> && ...))                              //
    {
        ((this->get<N>() = core::forward<Types>(rhs.template get<R>())), ...);
        return *this;
    }

    constexpr TupleImpl(const TupleImpl&) noexcept = default;

    template <usize... R, typename... Types>
    constexpr TupleImpl(
        const TupleImpl<std::integer_sequence<usize, R...>, Types...>& rhs)           //
        noexcept(std::conjunction_v<std::is_nothrow_assignable<Ts, const Types&>...>) //
        requires(                                                                     //
            (sizeof...(Ts) == sizeof...(Types))                                       //
            && (std::is_constructible_v<Ts, const Types&> && ...)                     //
            && !(std::is_same_v<Ts, Types> && ...))                                   //
        : TupleElement<N, Ts>(rhs.template get<R>())...
    {
    }

    constexpr TupleImpl& operator=(const TupleImpl&) noexcept = default;

    template <usize... R, typename... Types>
    constexpr TupleImpl& operator=(
        const TupleImpl<std::integer_sequence<usize, R...>, Types...>& rhs)           //
        noexcept(std::conjunction_v<std::is_nothrow_assignable<Ts, const Types&>...>) //
        requires(                                                                     //
            (sizeof...(Ts) == sizeof...(Types))                                       //
            && (std::is_assignable_v<Ts, const Types&> && ...)                        //
            && !(std::is_same_v<Ts, Types> && ...))                                   //
    {
        ((this->template get<N>() = rhs.template get<R>()), ...);
        return *this;
    }

    constexpr ~TupleImpl() noexcept = default;

public:
    template <usize I>
    [[nodiscard]] constexpr Type<I>& get() noexcept
    {
        static_assert(I < sizeof...(Ts), "Tuple index out of range!");
        return TupleElement<I, Type<I>>::get();
    }

    template <usize I>
    [[nodiscard]] constexpr const Type<I>& get() const noexcept
    {
        static_assert(I < sizeof...(Ts), "Tuple index out of range!");
        return TupleElement<I, Type<I>>::get();
    }

    template <typename T>
    [[nodiscard]] constexpr T& get() noexcept requires(!HAS_DUPLICATES)
    {
        constexpr auto index = core::TypeIndexOf<T, Types>::value;
        return this->template get<index>();
    }

    template <typename T>
    [[nodiscard]] constexpr const T& get() const noexcept requires(!HAS_DUPLICATES)
    {
        constexpr auto index = core::TypeIndexOf<T, Types>::value;
        return this->template get<index>();
    }
};

} // namespace tuple_impl

template <typename... Ts>
class Tuple
    : public tuple_impl::TupleImpl<std::make_integer_sequence<usize, sizeof...(Ts)>, Ts...> {
private:
    using Super =
        tuple_impl::TupleImpl<std::make_integer_sequence<usize, sizeof...(Ts)>, Ts...>;
    template <typename... Types>
    friend class Tuple;

public:
    constexpr Tuple() noexcept = default;
    template <typename... Args>
    constexpr Tuple(Args&&... args) //
        requires(
            (!(std::is_same_v<std::decay_t<Args>, Tuple> && ...)) //
            && (sizeof...(Args) == sizeof...(Ts))                 //
            && (std::is_constructible_v<Ts, Args> && ...))
        : Super { core::forward<Args>(args)... }
    {
    }

    constexpr Tuple(Tuple&&) noexcept = default;

    template <typename... Types>
    constexpr Tuple(Tuple<Types...>&& rhs)                                          //
        noexcept(std::conjunction_v<std::is_nothrow_constructible<Ts, Types&&>...>) //
        requires(                                                                   //
            (sizeof...(Ts) == sizeof...(Types))                                     //
            && (std::is_constructible_v<Ts, Types&&> && ...)                        //
            && !(std::is_same_v<Ts, Types> && ...))                                 //
        : Super(static_cast<typename Tuple<Types...>::Super&&>(rhs))
    {
    }

    constexpr Tuple& operator=(Tuple&&) //
        noexcept(std::conjunction_v<std::is_nothrow_move_assignable<Ts>...>) = default;

    template <typename... Types>
    constexpr Tuple& operator=(Tuple<Types...>&& rhs) //
        noexcept                                      //
        requires(
            (sizeof...(Ts) == sizeof...(Types))           //
            && (std::is_assignable_v<Ts, Types&&> && ...) //
            && !(std::is_same_v<Ts, Types> && ...))
    {
        static_cast<Super&>(*this) = static_cast<typename Tuple<Types...>::Super&&>(rhs);
        return *this;
    }

    constexpr Tuple(const Tuple&) //
        noexcept(std::conjunction_v<std::is_nothrow_copy_constructible<Ts>...>) = default;

    template <typename... Types>
    constexpr Tuple(const Tuple<Types...>& rhs) //
        noexcept(std::conjunction_v<std::is_nothrow_constructible<Ts, const Types&>...>) //
        requires(                                                 //
            (sizeof...(Ts) == sizeof...(Types))                   //
            && (std::is_constructible_v<Ts, const Types&> && ...) //
            && !(std::is_same_v<Ts, Types> && ...))               //
        : Super(static_cast<const typename Tuple<Types...>::Super&>(rhs))
    {
    }

    constexpr Tuple& operator=(const Tuple&) //
        noexcept(std::conjunction_v<std::is_nothrow_copy_assignable<Ts>...>) = default;

    template <typename... Types>
    constexpr Tuple& operator=(const Tuple<Types...>& rhs)                            //
        noexcept(std::conjunction_v<std::is_nothrow_assignable<Ts, const Types&>...>) //
        requires(
            (sizeof...(Ts) == sizeof...(Types))                //
            && (std::is_assignable_v<Ts, const Types&> && ...) //
            && !(std::is_same_v<Ts, Types> && ...))
    {
        static_cast<Super&>(*this) = static_cast<const typename Tuple<Types...>::Super&>(
            rhs);

        return *this;
    }

public:
    [[nodiscard]] constexpr usize size() const noexcept
    {
        return sizeof...(Ts);
    }
};

// Deduction Guides

template <typename... Types>
Tuple(Types&&...) -> Tuple<Types...>;

/////////////////////////////////////////////////////////////////////////////////////////
// Operators

template <typename... LhsTypes, typename... RhsTypes>
[[nodiscard]] constexpr bool operator==(
    const Tuple<LhsTypes...>& lhs, const Tuple<RhsTypes...>& rhs) noexcept
{
    static_assert(
        sizeof...(LhsTypes) == sizeof...(RhsTypes),
        "Cannot compare tuples of different sizes!");
    constexpr usize TUPLE_SIZE = sizeof...(LhsTypes);

    return [&]<typename... Lhs, typename... Rhs, usize... I>(
        const Tuple<Lhs...>& lhs,
        const Tuple<Rhs...>& rhs,
        [[maybe_unused]] std::index_sequence<I...>)
    {
        return (
            (static_cast<const tuple_impl::TupleElement<I, Lhs>&>(lhs) ==
             static_cast<const tuple_impl::TupleElement<I, Rhs>&>(rhs)) &&
            ...);
    }
    (lhs, rhs, std::make_index_sequence<TUPLE_SIZE> {});
}

template <typename... LhsTypes, typename... RhsTypes>
[[nodiscard]] constexpr bool operator!=(
    const Tuple<LhsTypes...>& lhs, const Tuple<RhsTypes...>& rhs) noexcept
{
    return !(lhs == rhs);
}

template <typename... LhsTypes, typename... RhsTypes>
[[nodiscard]] constexpr bool operator<(
    const Tuple<LhsTypes...>& lhs, const Tuple<RhsTypes...>& rhs) noexcept
{
    static_assert(
        sizeof...(LhsTypes) == sizeof...(RhsTypes),
        "Cannot compare tuples of different sizes!");
    constexpr usize TUPLE_SIZE = sizeof...(LhsTypes);

    return [&]<typename... Lhs, typename... Rhs, usize... I>(
        const Tuple<Lhs...>& lhs,
        const Tuple<Rhs...>& rhs,
        [[maybe_unused]] std::index_sequence<I...>)
    {
        return (
            (static_cast<const tuple_impl::TupleElement<I, Lhs>&>(lhs) <
             static_cast<const tuple_impl::TupleElement<I, Rhs>&>(rhs)) &&
            ...);
    }
    (lhs, rhs, std::make_index_sequence<TUPLE_SIZE> {});
}

template <typename... LhsTypes, typename... RhsTypes>
[[nodiscard]] constexpr bool operator<=(
    const Tuple<LhsTypes...>& lhs, const Tuple<RhsTypes...>& rhs) noexcept
{
    return !(lhs < rhs);
}

template <typename... LhsTypes, typename... RhsTypes>
[[nodiscard]] constexpr bool operator>(
    const Tuple<LhsTypes...>& lhs, const Tuple<RhsTypes...>& rhs) noexcept
{
    static_assert(
        sizeof...(LhsTypes) == sizeof...(RhsTypes),
        "Cannot compare tuples of different sizes!");
    constexpr usize TUPLE_SIZE = sizeof...(LhsTypes);

    return [&]<typename... Lhs, typename... Rhs, usize... I>(
        const Tuple<Lhs...>& lhs,
        const Tuple<Rhs...>& rhs,
        [[maybe_unused]] std::index_sequence<I...>)
    {
        return (
            (static_cast<const tuple_impl::TupleElement<I, Lhs>&>(lhs) >
             static_cast<const tuple_impl::TupleElement<I, Rhs>&>(rhs)) &&
            ...);
    }
    (lhs, rhs, std::make_index_sequence<TUPLE_SIZE> {});
}

template <typename... LhsTypes, typename... RhsTypes>
[[nodiscard]] constexpr bool operator>=(
    const Tuple<LhsTypes...>& lhs, const Tuple<RhsTypes...>& rhs) noexcept
{
    return !(lhs > rhs);
}

template <usize I, typename Tuple>
[[nodiscard]] constexpr decltype(auto) get(Tuple&& tuple) noexcept
{
    return core::move(tuple.template get<I>());
}

template <usize I, typename Tuple>
[[nodiscard]] constexpr decltype(auto) get(Tuple& tuple) noexcept
{
    return tuple.template get<I>();
}

template <usize I, typename Tuple>
[[nodiscard]] constexpr decltype(auto) get(const Tuple& tuple) noexcept
{
    return tuple.template get<I>();
}

/// This overload can be used only when there are no duplicate types in a tuple.
template <typename T, typename Tuple>
[[nodiscard]] constexpr decltype(auto) get(Tuple&& tuple) //
    noexcept                                              //
    requires(!Tuple::HAS_DUPLICATES)                      //
{
    return core::move(tuple.template get<T>());
}

/// This overload can be used only when there are no duplicate types in a tuple.
template <typename T, typename Tuple>
[[nodiscard]] constexpr decltype(auto) get(Tuple& tuple) //
    noexcept                                             //
    requires(!Tuple::HAS_DUPLICATES)                     //
{
    return tuple.template get<T>();
}

/// This overload can be used only when there are no duplicate types in a tuple.
template <typename T, typename Tuple>
[[nodiscard]] constexpr decltype(auto) get(const Tuple& tuple) //
    noexcept                                                   //
    requires(!Tuple::HAS_DUPLICATES)                           //
{
    return tuple.template get<T>();
}

template <typename... Args>
[[nodiscard]] constexpr Tuple<std::decay_t<Args>...> make_tuple(Args&&... args)
{
    return Tuple<std::decay_t<Args>...>(core::forward<Args>(args)...);
}

template <typename... Args>
[[nodiscard]] constexpr Tuple<Args&&...> forward_as_tuple(Args&&... args) noexcept
{
    return Tuple<Args&&...>(core::forward<Args>(args)...);
}

template <typename... Args>
[[nodiscard]] constexpr Tuple<Args&...> tie(Args&... args) noexcept
{
    return Tuple<Args&...>(args...);
}

template <typename Tuple, typename Func>
constexpr decltype(auto) for_each(Tuple&& tuple, Func&& func) //
    requires(concepts::has_tuple_size<std::decay_t<Tuple>>)
{
    return []<usize... I>(
        Tuple && tuple, Func && func, [[maybe_unused]] std::index_sequence<I...>)
    {
        (func(core::get<I>(tuple)), ...);
        return func;
    }
    (core::forward<Tuple>(tuple),
     core::forward<Func>(func),
     std::make_index_sequence<std::decay_t<Tuple>::TUPLE_SIZE> {});
}

template <typename... Types>
struct Hash<Tuple<Types...>> {
    [[nodiscard]] usize operator()(const Tuple<Types...>& v) const noexcept
    {
        return []<usize... I>(
            const Tuple<Types...>& v, [[maybe_unused]] std::index_sequence<I...>)
        {
            usize seed = 0;
            (core::hash_and_combine(seed, core::get<I>(v)), ...);
            return seed;
        }
        (v, std::make_index_sequence<sizeof...(Types)> {});
    }
};

} // namespace tundra::core

// Necessary for structured bindings.
namespace std {

template <typename>
struct tuple_size;

template <typename... Ts>
struct tuple_size<tundra::core::Tuple<Ts...>>
    : std::integral_constant<usize, sizeof...(Ts)> {
};

template <size_t I, typename>
struct tuple_element;

template <size_t I, typename... Ts>
struct tuple_element<I, tundra::core::Tuple<Ts...>> {
    using type = typename tundra::core::Tuple<Ts...>::template Type<I>;
};

} // namespace std
