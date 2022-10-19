#pragma once

#define TNDR_ENUM_CLASS_FLAGS(enumType)                                                  \
    constexpr enumType& operator|=(enumType& l, enumType r) noexcept                     \
    {                                                                                    \
        return l = static_cast<enumType>(                                                \
                   static_cast<__underlying_type(enumType)>(l) |                         \
                   static_cast<__underlying_type(enumType)>(r));                         \
    }                                                                                    \
    constexpr enumType& operator&=(enumType& l, enumType r) noexcept                     \
    {                                                                                    \
        return l = static_cast<enumType>(                                                \
                   static_cast<__underlying_type(enumType)>(l) &                         \
                   static_cast<__underlying_type(enumType)>(r));                         \
    }                                                                                    \
    constexpr enumType& operator^=(enumType& l, enumType r) noexcept                     \
    {                                                                                    \
        return l = static_cast<enumType>(                                                \
                   static_cast<__underlying_type(enumType)>(l) ^                         \
                   static_cast<__underlying_type(enumType)>(r));                         \
    }                                                                                    \
    [[nodiscard]] constexpr enumType operator|(enumType l, enumType r) noexcept          \
    {                                                                                    \
        return static_cast<enumType>(                                                    \
            static_cast<__underlying_type(enumType)>(l) |                                \
            static_cast<__underlying_type(enumType)>(r));                                \
    }                                                                                    \
    [[nodiscard]] constexpr enumType operator&(enumType l, enumType r) noexcept          \
    {                                                                                    \
        return static_cast<enumType>(                                                    \
            static_cast<__underlying_type(enumType)>(l) &                                \
            static_cast<__underlying_type(enumType)>(r));                                \
    }                                                                                    \
    [[nodiscard]] constexpr enumType operator^(enumType l, enumType r) noexcept          \
    {                                                                                    \
        return static_cast<enumType>(                                                    \
            static_cast<__underlying_type(enumType)>(l) ^                                \
            static_cast<__underlying_type(enumType)>(r));                                \
    }                                                                                    \
    [[nodiscard]] constexpr enumType operator~(enumType e) noexcept                      \
    {                                                                                    \
        return static_cast<enumType>(~(static_cast<__underlying_type(enumType)>(e)));    \
    }                                                                                    \
    [[nodiscard]] constexpr bool operator!(enumType e) noexcept                          \
    {                                                                                    \
        return !(static_cast<__underlying_type(enumType)>(e));                           \
    }                                                                                    \
    [[nodiscard]] constexpr bool is_empty(enumType lhs) noexcept                         \
    {                                                                                    \
        return static_cast<__underlying_type(enumType)>(lhs) == 0;                       \
    }                                                                                    \
    [[nodiscard]] constexpr bool contains(enumType lhs, enumType rhs) noexcept           \
    {                                                                                    \
        const auto lhs_bits = static_cast<__underlying_type(enumType)>(lhs);             \
        const auto rhs_bits = static_cast<__underlying_type(enumType)>(rhs);             \
        return (lhs_bits & rhs_bits) == rhs_bits;                                        \
    }                                                                                    \
    [[nodiscard]] constexpr bool intersects(enumType lhs, enumType rhs) noexcept         \
    {                                                                                    \
        const auto lhs_bits = static_cast<__underlying_type(enumType)>(lhs);             \
        const auto rhs_bits = static_cast<__underlying_type(enumType)>(rhs);             \
        return !is_empty(static_cast<enumType>(lhs_bits & rhs_bits));                    \
    }                                                                                    \
    constexpr void clear(enumType& lhs, enumType flags) noexcept                         \
    {                                                                                    \
        lhs &= (~flags);                                                                 \
    }                                                                                    \
    constexpr void toggle(enumType& lhs, enumType flags) noexcept                        \
    {                                                                                    \
        lhs ^= flags;                                                                    \
    }                                                                                    \
    constexpr enumType& set(enumType& lhs, enumType flags) noexcept                      \
    {                                                                                    \
        return (lhs |= flags);                                                           \
    }                                                                                    \
    [[nodiscard]] constexpr auto to_bytes(enumType l) noexcept                           \
    {                                                                                    \
        return static_cast<__underlying_type(enumType)>(l);                              \
    }
