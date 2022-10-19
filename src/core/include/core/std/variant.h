#pragma once
#include "core/core.h"
#include "core/std/assert.h"
#include "core/std/hash.h"
#include "core/std/type_list.h"
#include "core/std/utils.h"
#include <concepts>
#include <type_traits>

namespace tundra::core {

///
struct Monostate {
};

/// #TODO: Maybe generate jump table, instead of using fold expressions to generate `if` per type?
template <typename... Ts>
class Variant {
private:
    static_assert(!(std::is_reference_v<Ts> || ...), "References are not allowed!");
    static_assert(sizeof...(Ts) > 0, "Variant must have at least 1 type!");
    static_assert(!(std::is_reference_v<Ts> || ...), "Variant does not allow references!");
    static_assert(!(std::is_void_v<Ts> || ...), "Variant does not allow `void` type!");
    static_assert(!(std::is_array_v<Ts> || ...), "Variant does not allow arrays!");
    static_assert(
        std::is_same_v<core::TypeList<Ts...>, core::UniqueTypeList<Ts...>>,
        "Duplicates are not allowed!");
    using VariantTypeList = TypeList<Ts...>;
    using VariantOverloadSet = core::OverloadSet<Ts...>;

    [[nodiscard]] static constexpr usize max_size() noexcept
    {
        const auto values = { sizeof(Ts)... };

        auto largest = *values.begin();
        for (const auto value : values) {
            if (largest < value) {
                largest = value;
            }
        }

        return largest;
    }

public:
    static constexpr i32 npos = -1;

private:
    alignas(Ts...) char m_storage[max_size()] {};
    i32 m_index = npos;

public:
    constexpr Variant() noexcept = default;

    template <
        typename T,
        typename OverloadType = core::OverloadResolutionT<T, VariantOverloadSet>>
    constexpr Variant(T&& t)                                       //
        noexcept(std::is_nothrow_constructible_v<T, OverloadType>) //
        requires(
            !std::is_same_v<std::decay_t<T>, Variant> &&
            std::is_constructible_v<OverloadType, T>)
    {
        constexpr usize type_index =
            core::TypeIndexOf<std::decay_t<OverloadType>, VariantTypeList>::value;
        static_assert(type_index != (usize(-1)), "`T` not found in TypeList!");

        this->set<OverloadType>(core::forward<T>(t));
    }

    constexpr Variant(const Variant& rhs)                                        //
        noexcept(std::conjunction_v<std::is_nothrow_copy_constructible<Ts>...>   //
                     && std::conjunction_v<std::is_nothrow_destructible<Ts>...>) //
        requires(std::is_copy_constructible_v<Ts>&&...)                          //
    {
        (
            [&] {
                constexpr const i32 type_index = static_cast<i32>(
                    core::TypeIndexOf<Ts, VariantTypeList>::value);
                if (type_index == rhs.index()) {
                    this->set<Ts>(rhs.get<Ts>());
                }
            }(),
            ...);
    }

    constexpr Variant& operator=(const Variant& rhs)                             //
        noexcept(std::conjunction_v<std::is_nothrow_copy_constructible<Ts>...>   //
                     && std::conjunction_v<std::is_nothrow_destructible<Ts>...>) //
        requires(std::is_copy_constructible_v<Ts>&&...)
    {
        if (&rhs != this) {
            if (rhs.is_valueless()) {
                this->reset();
            } else {
                (
                    [&] {
                        constexpr const i32 type_index = static_cast<i32>(
                            core::TypeIndexOf<Ts, VariantTypeList>::value);
                        if (type_index == rhs.index()) {
                            this->set<Ts>(rhs.get<Ts>());
                        }
                    }(),
                    ...);
            }
        }
        return *this;
    }

    constexpr Variant(Variant&& rhs)                                             //
        noexcept(std::conjunction_v<std::is_nothrow_move_constructible<Ts>...>   //
                     && std::conjunction_v<std::is_nothrow_destructible<Ts>...>) //
        requires(std::is_move_constructible_v<Ts>&&...)
    {
        (
            [&] {
                constexpr const i32 type_index = static_cast<i32>(
                    core::TypeIndexOf<Ts, VariantTypeList>::value);
                if (type_index == rhs.index()) {
                    this->set<Ts>(core::move(rhs.get<Ts>()));
                }
            }(),
            ...);
    }

    constexpr Variant& operator=(Variant&& rhs)                                  //
        noexcept(std::conjunction_v<std::is_nothrow_move_constructible<Ts>...>   //
                     && std::conjunction_v<std::is_nothrow_destructible<Ts>...>) //
        requires(std::is_move_constructible_v<Ts>&&...)
    {
        if (&rhs != this) {
            if (rhs.is_valueless()) {
                this->reset();
            } else {
                (
                    [&] {
                        constexpr const i32 type_index = static_cast<i32>(
                            core::TypeIndexOf<Ts, VariantTypeList>::value);
                        if (type_index == rhs.index()) {
                            this->set<Ts>(core::move(rhs.get<Ts>()));
                            rhs.reset();
                        }
                    }(),
                    ...);
            }
        }
        return *this;
    }

    template <
        typename T,
        typename OverloadType = core::OverloadResolutionT<T, VariantOverloadSet>>
    constexpr Variant& operator=(T&& t) //
        noexcept(std::is_nothrow_constructible_v<T, OverloadType>&&
                     std::conjunction_v<std::is_nothrow_destructible<Ts>...>) //
        requires(
            !std::is_same_v<std::decay_t<T>, Variant> &&
            std::is_constructible_v<OverloadType, T>)
    {
        constexpr usize type_index =
            core::TypeIndexOf<std::decay_t<OverloadType>, VariantTypeList>::value;
        static_assert(type_index != (usize(-1)), "`T` not found in TypeList!");

        if (this->index() == type_index) {
            this->get<OverloadType>() = core::forward<T>(t);
        } else {
            this->set<OverloadType>(core::forward<T>(t));
        }

        return *this;
    }

    constexpr ~Variant() noexcept(std::conjunction_v<std::is_nothrow_destructible<Ts>...>)
    {
        this->reset();
    }

private:
    void reset() noexcept(std::conjunction_v<std::is_nothrow_destructible<Ts>...>)
    {
        if constexpr (!(std::conjunction_v<std::is_trivially_destructible<Ts>...>)) {
            if (!this->is_valueless()) {
                (
                    [&] {
                        if (m_index == core::TypeIndexOf<Ts, VariantTypeList>::value) {
                            reinterpret_cast<Ts*>(&m_storage)->~Ts();
                        }
                    }(),
                    ...);
                m_index = npos;
            }
        }
    }

private:
    template <typename T, typename... Args>
    void set(Args&&... args)                                                     //
        noexcept(std::is_nothrow_constructible_v<T, Args...>                     //
                     && std::conjunction_v<std::is_nothrow_destructible<Ts>...>) //
    {
        constexpr usize type_index =
            core::TypeIndexOf<std::decay_t<T>, VariantTypeList>::value;
        static_assert(type_index != (usize(-1)), "`T` not found in TypeList!");

        this->reset();
        ::new (&m_storage) T(core::forward<Args>(args)...);
        m_index = static_cast<i32>(type_index);
    }

public:
    [[nodiscard]] constexpr i32 index() const noexcept
    {
        return m_index;
    }

public:
    [[nodiscard]] constexpr bool is_valueless() const noexcept
    {
        return this->index() == npos;
    }

private:
    template <typename T>
    [[nodiscard]] constexpr T& get() noexcept
    {
        return *core::launder(reinterpret_cast<T*>(&m_storage));
    }

    template <typename T>
    [[nodiscard]] constexpr const T& get() const noexcept
    {
        return *core::launder(reinterpret_cast<const T*>(&m_storage));
    }

public:
    template <typename T, typename... Types>
    friend constexpr T& get(Variant<Types...>& variant) noexcept;
    template <typename T, typename... Types>
    friend constexpr T&& get(Variant<Types...>&& variant) noexcept;
    template <typename T, typename... Types>
    friend constexpr const T& get(const Variant<Types...>& variant) noexcept;

    template <usize I, typename... Types>
    friend constexpr TypeAtT<I, TypeList<Types...>>& get(
        Variant<Types...>& variant) noexcept;
    template <usize I, typename... Types>
    friend constexpr TypeAtT<I, TypeList<Types...>>&& get(
        Variant<Types...>&& variant) noexcept;
    template <usize I, typename... Types>
    friend constexpr const TypeAtT<I, TypeList<Types...>>& get(
        const Variant<Types...>& variant) noexcept;
};

/////////////////////////////////////////////////////////////////////////////////////////
// Get

template <typename T, typename... Types>
[[nodiscard]] constexpr T& get(Variant<Types...>& variant) noexcept
{
    using List = TypeList<Types...>;
    using Type = std::decay_t<T>;
    constexpr usize type_index = core::TypeIndexOf<Type, List>::value;
    static_assert(type_index < sizeof...(Types), "T is not a valid type!");

    tndr_assert(variant.index() == type_index, "Variant contains different type!");
    return variant.template get<Type>();
}

template <typename T, typename... Types>
[[nodiscard]] constexpr T&& get(Variant<Types...>&& variant) noexcept
{
    using List = TypeList<Types...>;
    using Type = std::decay_t<T>;
    constexpr usize type_index = core::TypeIndexOf<Type, List>::value;
    static_assert(type_index < sizeof...(Types), "T is not a valid type!");

    tndr_assert(variant.index() == type_index, "Variant contains different type!");
    return core::move(variant.template get<Type>());
}

template <typename T, typename... Types>
[[nodiscard]] constexpr const T& get(const Variant<Types...>& variant) noexcept
{
    using List = TypeList<Types...>;
    using Type = std::decay_t<T>;
    constexpr usize type_index = core::TypeIndexOf<Type, List>::value;
    static_assert(type_index < sizeof...(Types), "T is not a valid type!");

    tndr_assert(variant.index() == type_index, "Variant contains different type!");
    return variant.template get<Type>();
}

template <usize I, typename... Types>
[[nodiscard]] constexpr TypeAtT<I, TypeList<Types...>>& get(
    Variant<Types...>& variant) noexcept
{
    using List = TypeList<Types...>;

    static_assert(I < sizeof...(Types), "I is not a valid index!");
    tndr_assert(variant.index() == I, "Variant contains different type!");

    return variant.template get<TypeAtT<I, List>>();
}

template <usize I, typename... Types>
[[nodiscard]] constexpr TypeAtT<I, TypeList<Types...>>&& get(
    Variant<Types...>&& variant) noexcept
{
    using List = TypeList<Types...>;

    static_assert(I < sizeof...(Types), "I is not a valid index!");
    tndr_assert(variant.index() == I, "Variant contains different type!");

    return core::move(variant.template get<TypeAtT<I, List>>());
}

template <usize I, typename... Types>
[[nodiscard]] constexpr const TypeAtT<I, TypeList<Types...>>& get(
    const Variant<Types...>& variant) noexcept
{
    using List = TypeList<Types...>;

    static_assert(I < sizeof...(Types), "I is not a valid index!");
    tndr_assert(variant.index() == I, "Variant contains different type!");

    return variant.template get<TypeAtT<I, List>>();
}

template <typename T, typename... Types>
[[nodiscard]] constexpr bool holds_alternative(const Variant<Types...>& v) noexcept
{
    using List = TypeList<Types...>;
    constexpr usize type_index = core::TypeIndexOf<T, List>::value;
    return v.index() == static_cast<i32>(type_index);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Visit

namespace visit_private {

template <typename T, usize Size>
struct Array {
    T data[Size];
};

template <typename... Args>
[[nodiscard]] constexpr auto to_array(Args&&... args) noexcept
{
    Array<std::common_type_t<Args...>, sizeof...(Args)> arr {
        core::forward<Args>(args)...,
    };
    return arr;
}

template <typename Func, typename V, usize Idx>
struct DispatchHelper {
    static constexpr decltype(auto) dispatch(Func func, V&& variant)
    {
        return core::invoke(func, core::get<Idx>(variant));
    }
};

template <typename Func, typename Types, typename V, usize... Is>
[[nodiscard]] constexpr auto make_function_table_impl(
    [[maybe_unused]] std::index_sequence<Is...>) noexcept
{
    return to_array(&DispatchHelper<Func, V, Is>::dispatch...);
}

template <typename Func, typename Types, typename V>
[[nodiscard]] constexpr auto make_function_table() noexcept
{
    return make_function_table_impl<Func, Types, V>(
        std::make_index_sequence<Types::count()> {});
}

} // namespace visit_private

// https://mpark.github.io/programming/2019/01/22/variant-visitation-v2/
template <typename Visitor, typename V>
[[nodiscard]] constexpr decltype(auto) visit(Visitor&& visitor, V&& variant)
{
    using DecayedVariant = std::decay_t<V>;

    constexpr auto table = visit_private::
        make_function_table<Visitor, core::Rename<DecayedVariant, TypeList>, V>();
    tndr_assert(variant.index() != DecayedVariant::npos, "Variant is in invalid state!");
    return table.data[variant.index()](
        core::forward<Visitor>(visitor), core::forward<V>(variant));
}

/////////////////////////////////////////////////////////////////////////////////////////
// Comparisons

namespace compare_impl {

template <typename V, usize Idx>
struct CompareHelper {
    static constexpr bool dispatch(const V& lhs, const V& rhs) noexcept
    {
        return core::get<Idx>(lhs) == core::get<Idx>(rhs);
    }
};

template <typename... Ts, usize... Is>
[[nodiscard]] constexpr auto make_function_table_impl(
    [[maybe_unused]] std::index_sequence<Is...>) noexcept
{
    using V = Variant<Ts...>;
    return visit_private::to_array(&CompareHelper<V, Is>::dispatch...);
}

template <typename... Ts>
[[nodiscard]] constexpr auto make_function_table() noexcept
{
    return make_function_table_impl<Ts...>(std::make_index_sequence<sizeof...(Ts)> {});
}

} // namespace compare_impl

template <typename... Ts>
[[nodiscard]] constexpr bool operator==(
    const Variant<Ts...>& lhs, const Variant<Ts...>& rhs) noexcept //
    requires(std::equality_comparable<Ts>&&...)
{
    if ((lhs.index() == rhs.index()) && !lhs.is_valueless()) {
        constexpr auto table = compare_impl::make_function_table<Ts...>();
        return table.data[lhs.index()](lhs, rhs);
    }
    return false;
}

template <typename... Ts>
[[nodiscard]] constexpr bool operator!=(
    const Variant<Ts...>& lhs, const Variant<Ts...>& rhs) noexcept //
    requires(std::equality_comparable<Ts>&&...)
{
    return !(lhs == rhs);
}

template <typename... Types>
struct Hash<Variant<Types...>> {
    [[nodiscard]] usize operator()(const Variant<Types...>& variant) const noexcept
    {
        return core::visit(
            []<typename T>(const T& value) { return core::Hash<T> {}(value); }, variant);
    }
};

} // namespace tundra::core
