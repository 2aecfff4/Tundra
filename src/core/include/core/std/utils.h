#pragma once
#include <type_traits>

namespace tundra::core {

/// This is equivalent of std::move, but this version won't compile when rvalue or const reference object is passed.
template <typename T>
[[nodiscard]] constexpr std::remove_reference_t<T>&& move(T&& object) noexcept
{
    using Type = std::remove_reference_t<T>;
    static_assert(!std::is_same_v<Type&, const Type&>, "Move called on const object.");
    static_assert(std::is_lvalue_reference_v<T>, "Move called on an rvalue");
    return static_cast<Type&&>(object);
}

/// Same as std::forward
template <typename T>
[[nodiscard]] constexpr T&& forward(std::remove_reference_t<T>& arg) noexcept
{
    return static_cast<T&&>(arg);
}

template <typename T>
[[nodiscard]] constexpr T&& forward(std::remove_reference_t<T>&& arg) noexcept
{
    static_assert(!std::is_lvalue_reference_v<T>, "Bad forward call!");
    // Forward an rvalue as an rvalue.
    return static_cast<T&&>(arg);
}

template <typename T>
constexpr void swap(T& left, T& right) //
    noexcept(
        std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_move_assignable_v<T>) //
    requires(std::is_move_constructible_v<T>&& std::is_move_assignable_v<T>)
{
    T temp = core::move(left);
    left = core::move(right);
    right = core::move(temp);
}

template <typename T, typename R = T>
[[nodiscard]] constexpr T exchange(T& value, R&& new_value) //
    noexcept(std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_assignable_v<T&, R>)
{
    T old_value = core::move(value);
    value = core::forward<R>(new_value);
    return old_value;
}

template <typename T>
[[nodiscard]] constexpr T* launder(T* ptr) noexcept
{
    static_assert(!std::is_function_v<T> && !std::is_void_v<T>);
    return __builtin_launder(ptr);
}

template <typename T>
[[nodiscard]] constexpr std::add_const_t<T>& as_const(T& value) noexcept
{
    return value;
}

template <typename T>
void as_const(const T&&) = delete;

///
template <typename T>
[[nodiscard]] consteval auto as_consteval(T&& value) noexcept
{
    return core::forward<T>(value);
}

template <typename To, typename From>
[[nodiscard]] constexpr To bit_cast(const From& v) noexcept
    requires(std::conjunction_v<
             std::bool_constant<sizeof(To) == sizeof(From)>,
             std::is_trivially_copyable<To>,
             std::is_trivially_copyable<From>>)
{
    return __builtin_bit_cast(To, v);
}

template <typename T>
[[nodiscard]] constexpr T* addressof(T& value) noexcept
{
    return __builtin_addressof(value);
}

template <typename T>
const T* addressof(const T&&) = delete;

namespace overload_impl {

template <typename... Ts>
struct Overload : public Ts... {
    using Ts::operator()...;
};

template <typename... Ts>
Overload(Ts...) -> Overload<Ts...>;

} // namespace overload_impl

template <typename... Ts>
[[nodiscard]] constexpr auto make_overload(Ts&&... lambdas) noexcept
{
    return overload_impl::Overload { core::forward<Ts>(lambdas)... };
}

//////////////////////////////////////////////////////////////////////////
// Invoke

namespace invoke_private {

template <typename>
inline constexpr bool is_reference_wrapper = false;
template <typename T>
inline constexpr bool is_reference_wrapper<std::reference_wrapper<T>> = true;

template <typename T, typename Type, typename T1, typename... Args>
[[nodiscard]] constexpr decltype(auto) invoke_impl(Type T::*f, T1&& t1, Args&&... args)
{
    if constexpr (std::is_member_function_pointer_v<decltype(f)>) {
        if constexpr (std::is_base_of_v<T, std::decay_t<T1>>) {
            return (core::forward<T1>(t1).*f)(core::forward<Args>(args)...);
        } else if constexpr (is_reference_wrapper<std::decay_t<T1>>) {
            return (t1.get().*f)(core::forward<Args>(args)...);
        } else {
            return ((*core::forward<T1>(t1)).*f)(core::forward<Args>(args)...);
        }
    } else {
        static_assert(std::is_member_object_pointer_v<decltype(f)>);
        static_assert(sizeof...(args) == 0);
        if constexpr (std::is_base_of_v<T, std::decay_t<T1>>) {
            return core::forward<T1>(t1).*f;
        } else if constexpr (is_reference_wrapper<std::decay_t<T1>>) {
            return t1.get().*f;
        } else {
            return (*core::forward<T1>(t1)).*f;
        }
    }
}

template <typename F, typename... Args>
[[nodiscard]] constexpr decltype(auto) invoke_impl(F&& f, Args&&... args)
{
    return core::forward<F>(f)(core::forward<Args>(args)...);
}

} // namespace invoke_private

template <typename F, typename... Args>
[[nodiscard]] constexpr std::invoke_result_t<F, Args...> invoke(
    F&& f, Args&&... args) noexcept(std::is_nothrow_invocable_v<F, Args...>)
{
    return invoke_private::invoke_impl(core::forward<F>(f), core::forward<Args>(args)...);
}

} // namespace tundra::core
