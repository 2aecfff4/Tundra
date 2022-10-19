#pragma once
#include "core/core.h"
#include "core/std/assert.h"
#include "core/std/traits/declval.h"
#include "core/std/utils.h"
#include "traits/is_callable.h"
#include <type_traits>

namespace tundra::core {

///
template <typename T, typename E>
class Expected;

///
template <typename E>
class Unexpected {
    static_assert(!std::is_void_v<E>, "E must not be void");

public:
    Unexpected() noexcept = delete;
    constexpr explicit Unexpected(const E& v)
        : m_val(v)
    {
    }

    constexpr explicit Unexpected(E&& v)
        : m_val(core::move(v))
    {
    }

    [[nodiscard]] constexpr const E& value() const&
    {
        return m_val;
    }

    [[nodiscard]] constexpr E& value() &
    {
        return m_val;
    }

    [[nodiscard]] constexpr E&& value() &&
    {
        return core::move(m_val);
    }

    [[nodiscard]] constexpr E const&& value() const&&
    {
        return core::move(m_val);
    }

private:
    E m_val;
};

///
template <typename E>
[[nodiscard]] Unexpected<std::decay_t<E>> make_unexpected(E&& e)
{
    return Unexpected<std::decay_t<E>>(core::forward<E>(e));
}

namespace expected_impl {

///
template <typename R, typename E>
class Storage {
public:
    enum class State : u8 {
        Ok,
        Err,
        Invalid,
    };

    template <typename T>
    static constexpr const T& max(const T& a, const T& b) noexcept
    {
        return (a < b) ? b : a;
    }

    template <typename Lhs, typename Rhs>
    static constexpr usize ALIGNMENT = max(alignof(Lhs), alignof(Rhs));

    template <typename Rhs>
    static constexpr usize ALIGNMENT<void, Rhs> = alignof(Rhs);

    template <typename Lhs, typename Rhs>
    static constexpr usize SIZE = max(sizeof(Lhs), sizeof(Rhs));

    template <typename Rhs>
    static constexpr usize SIZE<void, Rhs> = sizeof(Rhs);

private:
    alignas(ALIGNMENT<R, E>) char m_buffer[SIZE<R, E>];
    State m_state = State::Invalid;

public:
    // Destructor
    constexpr ~Storage() noexcept = default;
    constexpr ~Storage() noexcept //
        requires(
            !std::is_trivially_destructible_v<R> ||
            !std::is_trivially_destructible_v<E>) //
    {
        switch (m_state) {
            case State::Ok: {
                this->destroy_value();
                break;
            }
            case State::Err: {
                this->destroy_err();
                break;
            }
        }
    }

public:
    template <typename... Args>
    constexpr void construct_value(Args&&... args)
    {
        if constexpr (!std::is_void_v<R>) {
            new (m_buffer) R(core::forward<Args>(args)...);
        }
        m_state = State::Ok;
    }

    constexpr void destroy_value()
    {
        if constexpr (!std::is_trivially_destructible_v<R> && !std::is_void_v<R>) {
            core::launder(reinterpret_cast<R*>(m_buffer))->~R();
        }
        m_state = State::Invalid;
    }

public:
    template <typename... Args>
    constexpr void construct_err(Args&&... args)
    {
        new (m_buffer) E(core::forward<Args>(args)...);
        m_state = State::Err;
    }

    constexpr void destroy_err()
    {
        if constexpr (!std::is_trivially_destructible_v<E>) {
            core::launder(reinterpret_cast<E*>(m_buffer))->~E();
        }
        m_state = State::Invalid;
    }

public:
    template <typename U = R>
    [[nodiscard]] constexpr const U& get_value() const& noexcept //
        requires(!std::is_void_v<U>)
    {
        return *core::launder(reinterpret_cast<const R*>(m_buffer));
    }

    template <typename U = R>
    [[nodiscard]] constexpr U& get_value() & noexcept //
        requires(!std::is_void_v<U>)
    {
        return *core::launder(reinterpret_cast<R*>(m_buffer));
    }

    template <typename U = R>
    [[nodiscard]] constexpr U&& get_value() && noexcept //
        requires(!std::is_void_v<U>)
    {
        return core::move(*core::launder(reinterpret_cast<R*>(m_buffer)));
    }

    template <typename U = R>
    [[nodiscard]] constexpr U&& get_value() const&& noexcept //
        requires(!std::is_void_v<U>)
    {
        return core::move(*core::launder(reinterpret_cast<R*>(m_buffer)));
    }

    [[nodiscard]] constexpr const E& get_err() const& noexcept
    {
        return *core::launder(reinterpret_cast<const E*>(m_buffer));
    }

    [[nodiscard]] constexpr E& get_err() & noexcept
    {
        return *core::launder(reinterpret_cast<E*>(m_buffer));
    }

    [[nodiscard]] constexpr E&& get_err() && noexcept
    {
        return core::move(*core::launder(reinterpret_cast<E*>(m_buffer)));
    }

    [[nodiscard]] constexpr E&& get_err() const&& noexcept
    {
        return core::move(*core::launder(reinterpret_cast<E*>(m_buffer)));
    }

public:
    [[nodiscard]] constexpr bool has_value() const noexcept
    {
        return m_state == State::Ok;
    }
};

} // namespace expected_impl

/// Based on http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0323r3.pdf
template <typename T, typename E>
class Expected {
    static_assert(!std::is_void_v<E>, "`E` must not be void.");
    static_assert(!std::is_reference_v<T>, "`T` must not be a reference.");
    static_assert(!std::is_reference_v<E>, "`E` must not be a reference.");
    static_assert(
        !std::is_same_v<T, std::remove_cv_t<Unexpected<E>>>,
        "`T` must not be `Unexpected<E>`.");

public:
    using ValueType = T;
    using ErrorType = E;

private:
    expected_impl::Storage<T, Unexpected<E>> m_storage;

public:
    constexpr Expected() //
        requires(std::is_default_constructible_v<T> || std::is_void_v<T>)
    {
        m_storage.construct_value();
    }

    constexpr Expected(const Expected& rhs) //
        requires(
            (std::is_copy_constructible_v<T> ||
             std::is_void_v<T>)&&std::is_copy_constructible_v<E>)
    {
        if (rhs.has_value()) {
            m_storage.construct_value(rhs.value());
        } else {
            m_storage.construct_err(rhs.error());
        }
    }

    constexpr Expected(Expected&& rhs) //
        noexcept(
            (std::is_nothrow_move_constructible_v<T> ||
             std::is_void_v<T>)&&std::is_nothrow_move_constructible_v<E>)           //
        requires(std::is_move_constructible_v<T>&& std::is_move_constructible_v<E>) //
    {
        if (rhs.has_value()) {
            m_storage.construct_value(core::move(rhs.value()));
        } else {
            m_storage.construct_err(core::move(rhs.error()));
        }
    }

    // clang-format off
    template <typename U, typename G>
    explicit(
        !std::is_void_v<T> &&
        (std::is_convertible_v<const U&, T> && std::is_convertible_v<const G&, E>)) //
        constexpr Expected(const Expected<U, G>& rhs)                               //
        requires(
            (std::is_constructible_v<T, const U&> || (std::is_void_v<T> && std::is_void_v<U>)) &&
            std::is_constructible_v<E, const G&> &&
            (!std::is_constructible_v<T, Expected<U,G>&> || (std::is_void_v<T> && std::is_void_v<U>)) &&
            (!std::is_constructible_v<T, Expected<U,G>&&> || (std::is_void_v<T> && std::is_void_v<U>)) &&
            (!std::is_constructible_v<T, const Expected<U,G>&> || (std::is_void_v<T> && std::is_void_v<U>)) &&
            (!std::is_constructible_v<T, const Expected<U,G>&&> || (std::is_void_v<T> && std::is_void_v<U>)) &&
            (!std::is_convertible_v<Expected<U,G>&, T> || (std::is_void_v<T> && std::is_void_v<U>)) &&
            (!std::is_convertible_v<Expected<U,G>&&, T> || (std::is_void_v<T> && std::is_void_v<U>)) &&
            (!std::is_convertible_v<const Expected<U,G>&, T> || (std::is_void_v<T> && std::is_void_v<U>)) &&
            (!std::is_convertible_v<const Expected<U,G>&&, T> || (std::is_void_v<T> && std::is_void_v<U>)))
    {
        // clang-format on
        if (rhs.has_value()) {
            m_storage.construct_value(rhs.value());
        } else {
            m_storage.construct_err(rhs.error());
        }
    }

    // clang-format off
    template <typename U, typename G>
    explicit(!std::is_convertible_v<U&&, T> || !std::is_convertible_v<G&&, E>) //
        constexpr Expected(Expected<U, G>&&rhs)
        requires(
            std::is_constructible_v<T, U&&> &&
            std::is_constructible_v<E, G&&> &&
            (!std::is_constructible_v<T, Expected<U,G>&> || (std::is_void_v<T> && std::is_void_v<U>)) &&
            (!std::is_constructible_v<T, Expected<U,G>&&> || (std::is_void_v<T> && std::is_void_v<U>)) &&
            (!std::is_constructible_v<T, const Expected<U,G>&> || (std::is_void_v<T> && std::is_void_v<U>)) &&
            (!std::is_constructible_v<T, const Expected<U,G>&&> || (std::is_void_v<T> && std::is_void_v<U>)) &&
            (!std::is_convertible_v<Expected<U,G>&, T> || (std::is_void_v<T> && std::is_void_v<U>)) &&
            (!std::is_convertible_v<Expected<U,G>&&, T> || (std::is_void_v<T> && std::is_void_v<U>)) &&
            (!std::is_convertible_v<const Expected<U,G>&, T> || (std::is_void_v<T> && std::is_void_v<U>)) &&
            (!std::is_convertible_v<const Expected<U,G>&&, T> || (std::is_void_v<T> && std::is_void_v<U>)))        
    {
        // clang-format on
        if (rhs.has_value()) {
            m_storage.construct_value(core::move(rhs.value()));
        } else {
            m_storage.construct_err(core::move(rhs.error()));
        }
    }

    template <typename U = T>
    explicit(!std::is_convertible_v<U&&, T>) //
        constexpr Expected(U&& v)            //
        requires(
            !std::is_void_v<T> && std::is_constructible_v<T, U&&> &&
            !std::is_same_v<Expected<T, E>, std::decay_t<U>> &&
            !std::is_same_v<Unexpected<E>, std::decay_t<U>>)
    {
        m_storage.construct_value(core::move(v));
    }

    template <typename G = E>
    explicit(!std::is_convertible_v<const G&, E>)    //
        constexpr Expected(const Unexpected<G>& rhs) //
        requires(std::is_constructible_v<E, const G&>)
    {
        m_storage.construct_err(rhs);
    }

    template <typename G = E>
    constexpr explicit(!std::is_convertible_v<G&&, E>)    //
        Expected(Unexpected<G>&& rhs)                     //
        noexcept(std::is_nothrow_constructible_v<E, G&&>) //
        requires(std::is_constructible_v<E, G&&>)
    {
        m_storage.construct_err(core::move(rhs.value()));
    }

    ~Expected() = default;

public:
    Expected& operator=(const Expected& rhs) //
        requires(
            !std::is_void_v<T> ||
            (std::is_copy_assignable_v<T> && std::is_copy_assignable_v<E> &&
             std::is_copy_constructible_v<E> && std::is_copy_constructible_v<T> &&
             std::is_nothrow_move_constructible_v<E>))
    {
        if (&rhs != this) {
            if (this->has_value() && rhs.has_value()) { // true && true
                m_storage.get_value() = rhs.m_storage.get_value();
            } else if (!this->has_value() && !rhs.has_value()) { // false && false
                m_storage.get_err().value() = rhs.m_storage.get_err().value();
            } else if (this->has_value() && !rhs.has_value()) { // true && false
                m_storage.destroy_value();
                m_storage.construct_err(rhs.error());
            } else { // false && true
                m_storage.destroy_err();
                m_storage.construct_value(rhs.value());
            }
        }

        return *this;
    }

    Expected& operator=(Expected&& rhs) //
        noexcept(std::is_nothrow_move_assignable_v<T>&&
                     std::is_nothrow_move_constructible_v<E>) //
        requires(std::is_move_constructible_v<T>&& std::is_move_assignable_v<T>&&
                     std::is_nothrow_move_constructible_v<E>&&
                         std::is_nothrow_move_assignable_v<E>)
    {
        if (&rhs != this) {
            if (this->has_value() && rhs.has_value()) { // true && true
                m_storage.get_value() = core::move(rhs.m_storage.get_value());
            } else if (!this->has_value() && !rhs.has_value()) { // false && false
                m_storage.get_err().value() = core::move(rhs.m_storage.get_err().value());
            } else if (this->has_value() && !rhs.has_value()) { // true && false
                m_storage.destroy_value();
                m_storage.construct_err(core::move(rhs.error()));
            } else { // false && true
                m_storage.destroy_err();
                m_storage.construct_value(core::move(rhs.value()));
            }
        }

        return *this;
    }

    template <typename U = T, typename TempT = T>
    Expected& operator=(U&& rhs) //
        requires(
            (!std::is_void_v<T> &&
             !std::is_void_v<TempT>)&&!std::is_same_v<Expected<T, E>, std::decay_t<U>> &&
            !std::conjunction_v<std::is_scalar<T>, std::is_same<T, std::decay_t<U>>> &&
            std::is_constructible_v<T, U> && std::is_assignable_v<TempT&, U> &&
            std::is_nothrow_move_constructible_v<E>)
    {
        if (this->has_value()) {
            m_storage.get_value() = core::forward<U>(rhs);
        } else {
            m_storage.destroy_err();
            m_storage.construct_value(core::forward<U>(rhs));
        }

        return *this;
    }

    template <typename G = E>
    Expected& operator=(const Unexpected<G>& rhs) //
        requires(
            std::is_nothrow_copy_constructible_v<G>&& std::is_assignable_v<E&, const G&>)
    {
        if (this->has_value()) {
            m_storage.destroy_value();
            m_storage.construct_err(rhs.value());
        } else {
            m_storage.get_err().value() = rhs.value();
        }

        return *this;
    }

    template <typename G = E>
    Expected& operator=(Unexpected<G>&& rhs) //
        requires(std::is_nothrow_move_constructible_v<G>&& std::is_assignable_v<E&, G&&>)
    {
        if (this->has_value()) {
            m_storage.destroy_value();
            m_storage.construct_err(core::move(rhs.value()));
        } else {
            m_storage.get_err().value() = core::move(rhs.value());
        }

        return *this;
    }

    // #TODO: Maybe?
    // void swap(Expected&) noexcept(expr);

public:
    template <typename U = T>
    [[nodiscard]] constexpr const U* operator->() const noexcept //
        requires(!std::is_void_v<U>)
    {
        tndr_assert(this->has_value(), "Bad expected access!");
        return &m_storage.get_value();
    }

    template <typename U = T>
    [[nodiscard]] constexpr U* operator->() noexcept //
        requires(!std::is_void_v<U>)
    {
        tndr_assert(this->has_value(), "Bad expected access!");
        return &m_storage.get_value();
    }

    template <typename U = T>
    [[nodiscard]] constexpr const U& operator*() const& noexcept //
        requires(!std::is_void_v<U>)
    {
        tndr_assert(this->has_value(), "Bad expected access!");
        return m_storage.get_value();
    }

    template <typename U = T>
    [[nodiscard]] constexpr U& operator*() & noexcept //
        requires(!std::is_void_v<U>)
    {
        tndr_assert(this->has_value(), "Bad expected access!");
        return m_storage.get_value();
    }

    template <typename U = T>
    [[nodiscard]] constexpr const U&& operator*() const&& noexcept //
        requires(!std::is_void_v<U>)
    {
        tndr_assert(this->has_value(), "Bad expected access!");
        return core::move(m_storage.get_value());
    }

    template <typename U = T>
    [[nodiscard]] constexpr U&& operator*() && noexcept //
        requires(!std::is_void_v<U>)
    {
        tndr_assert(this->has_value(), "Bad expected access!");
        return core::move(m_storage.get_value());
    }

    [[nodiscard]] constexpr explicit operator bool() const noexcept
    {
        return this->has_value();
    }

    [[nodiscard]] constexpr bool has_value() const noexcept
    {
        return m_storage.has_value();
    }

    template <typename U = T>
    [[nodiscard]] constexpr const U& value() const& noexcept //
        requires(!std::is_void_v<U>)
    {
        tndr_assert(this->has_value(), "Bad expected access!");
        return m_storage.get_value();
    }

    template <typename U = T>
    [[nodiscard]] constexpr U& value() & noexcept //
        requires(!std::is_void_v<U>)
    {
        tndr_assert(this->has_value(), "Bad expected access!");
        return m_storage.get_value();
    }

    template <typename U = T>
    [[nodiscard]] constexpr const U&& value() const&& noexcept //
        requires(!std::is_void_v<U>)
    {
        tndr_assert(this->has_value(), "Bad expected access!");
        return core::move(m_storage.get_value());
    }

    template <typename U = T>
    [[nodiscard]] constexpr U&& value() && noexcept //
        requires(!std::is_void_v<U>)
    {
        tndr_assert(this->has_value(), "Bad expected access!");
        return core::move(m_storage.get_value());
    }

    [[nodiscard]] constexpr const E& error() const&
    {
        tndr_assert(!this->has_value(), "Bad expected access!");
        return m_storage.get_err().value();
    }

    [[nodiscard]] constexpr E& error() &
    {
        tndr_assert(!this->has_value(), "Bad expected access!");
        return m_storage.get_err().value();
    }

    [[nodiscard]] constexpr const E&& error() const&&
    {
        tndr_assert(!this->has_value(), "Bad expected access!");
        return core::move(m_storage.get_err().value());
    }

    [[nodiscard]] constexpr E&& error() &&
    {
        tndr_assert(!this->has_value(), "Bad expected access!");
        return core::move(m_storage.get_err().value());
    }

    template <typename U, typename TestT = T>
    [[nodiscard]] constexpr T value_or(U&& v) const& //
        requires(!std::is_void_v<TestT>)
    {
        static_assert(
            std::is_move_constructible_v<T> && std::is_convertible_v<U&&, T>,
            "T must be move-constructible and convertible to from U&&");

        return this->has_value() ? this->value() : v;
    }

    template <typename U, typename TestT = T>
        [[nodiscard]] T value_or(U&& v) && //
        requires(!std::is_void_v<TestT>)
    {
        static_assert(
            std::is_move_constructible_v<T> && std::is_convertible_v<U&&, T>,
            "T must be move-constructible and convertible to from U&&");

        return this->has_value() ? core::move(this->value()) : v;
    }

    template <traits::callable<E> Func, typename TestT = T>
    [[nodiscard]] constexpr auto value_or_else(Func&& func) const& //
        requires(!std::is_void_v<T>)
    {
        using ReturnType = std::invoke_result_t<Func, E>;

        static_assert(
            std::is_move_constructible_v<T> && std::is_convertible_v<ReturnType&&, T>,
            "T must be move-constructible and convertible to from U&&");

        return this->has_value() ? this->value() : func(this->error());
    }

    template <traits::callable<E> Func, typename TestT = T>
        [[nodiscard]] auto value_or_else(Func&& func) && //
        requires(!std::is_void_v<T>)
    {
        using ReturnType = std::invoke_result_t<Func, E>;

        static_assert(
            std::is_move_constructible_v<T> && std::is_convertible_v<ReturnType&&, T>,
            "T must be move-constructible and convertible to from U&&");

        return this->has_value() ? core::move(this->value())
                                 : func(core::move(this->error()));
    }

    template <typename U>
    [[nodiscard]] constexpr E error_or(U&& v) const&
    {
        static_assert(
            std::is_move_constructible_v<E> && std::is_convertible_v<U&&, E>,
            "E must be move-constructible and convertible to from E&&");

        return !this->has_value() ? this->error() : v;
    }

    template <typename U>
    [[nodiscard]] E error_or(U&& v) &&
    {
        static_assert(
            std::is_move_constructible_v<E> && std::is_convertible_v<U&&, E>,
            "E must be move-constructible and convertible to from E&&");

        return !this->has_value() ? core::move(this->error()) : v;
    }

private:
    template <
        typename Exp,
        typename Func,
        typename Ret = decltype(core::invoke(
            core::traits::declval<Func>(), core::traits::declval<Exp>().error()))>
    [[nodiscard]] constexpr auto map_error_impl(Exp&& exp, Func&& f) //
        requires(!std::is_void_v<ValueType> && !std::is_void_v<Ret>)
    {
        using Result = Expected<ValueType, std::decay_t<Ret>>;

        return exp.has_value()
                   ? Result(*core::forward<Exp>(exp))
                   : Result(core::make_unexpected(core::invoke(
                         core::forward<Func>(f), core::forward<Exp>(exp).error())));
    }

public:
    template <typename Func>
    [[nodiscard]] constexpr auto map_error(Func&& f) &
    {
        return this->map_error_impl(*this, core::forward<Func>(f));
    }

    template <typename Func>
    [[nodiscard]] constexpr auto map_error(Func&& f) &&
    {
        return this->map_error_impl(core::move(*this), core::forward<Func>(f));
    }

    template <typename Func>
    [[nodiscard]] constexpr auto map_error(Func&& f) const&
    {
        return this->map_error_impl(*this, core::forward<Func>(f));
    }

    template <typename Func>
    [[nodiscard]] constexpr auto map_error(Func&& f) const&&
    {
        return this->map_error_impl(core::move(*this), core::forward<Func>(f));
    }
};

} // namespace tundra::core
