#pragma once
#include "core/core.h"
#include <concepts>
#include <type_traits>

namespace tundra::core {

// Based on boost container_hash and the work of Austin Appleby
namespace hash_impl {

///
[[nodiscard]] inline usize mumrmur_hash2(const void* key, const usize len) noexcept
{
    constexpr u64 m = 0xc6a4a7935bd1e995ull;
    constexpr u64 seed = 0xe17a1465;
    constexpr u64 r = 47;

    u64 h = seed ^ (len * m);

    const u64* data = reinterpret_cast<const u64*>(key);
    const u64* end = data + (len / 8);

    while (data != end) {
        u64 k = *data++;

        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;
    }

    const auto* data2 = reinterpret_cast<const unsigned char*>(data);
    switch (len & 7) {
        case 7:
            h ^= u64(data2[6]) << 48;
            [[fallthrough]];
        case 6:
            h ^= u64(data2[5]) << 40;
            [[fallthrough]];
        case 5:
            h ^= u64(data2[4]) << 32;
            [[fallthrough]];
        case 4:
            h ^= u64(data2[3]) << 24;
            [[fallthrough]];
        case 3:
            h ^= u64(data2[2]) << 16;
            [[fallthrough]];
        case 2:
            h ^= u64(data2[1]) << 8;
            [[fallthrough]];
        case 1:
            h ^= u64(data2[0]);
            h *= m;
            [[fallthrough]];
        default:
            break;
    };

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return static_cast<usize>(h);
}

///
constexpr void hash_combine_impl(u32& h1, u32 k1) noexcept
{
    const auto rotl = [](const u32 x, const u32 rotation) constexpr
    {
        return (x << rotation) | (x >> (32u - rotation));
    };

    const u32 c1 = 0xcc9e2d51;
    const u32 c2 = 0x1b873593;

    k1 *= c1;
    k1 = rotl(k1, 15);
    k1 *= c2;

    h1 ^= k1;
    h1 = rotl(h1, 13);
    h1 = h1 * 5 + 0xe6546b64;
}

///
constexpr void hash_combine_impl(u64& h, u64 k) noexcept
{
    const u64 m = 0xc6a4a7935bd1e995ull;
    const u64 r = 47;

    k *= m;
    k ^= k >> r;
    k *= m;

    h ^= k;
    h *= m;

    // Completely arbitrary number, to prevent 0's
    // from hashing to 0.
    h += 0xe6546b64;
}

///
template <typename T, bool Enable>
struct BaseHash {
    [[nodiscard]] usize operator()(const T& v) const noexcept
    {
        return hash_impl::mumrmur_hash2(&v, sizeof(T));
    }
};

///
template <typename T>
struct BaseHash<T, false> {
    static_assert(
        (std::is_enum_v<T> || std::is_integral_v<T> || std::is_pointer_v<T>),
        "Please add `Hash` specialization for `T`.");

    [[nodiscard]] usize operator()(const T& v) const noexcept = delete;
};

} // namespace hash_impl

template <typename T>
struct Hash;

/// Custom hash implementation. Internally uses murmurhash2.
///
/// Example:
/// ```
/// template <>
/// struct Hash<vulkan_rhi::FramebufferKey> {
///     [[nodiscard]] usize operator()(const vulkan_rhi::FramebufferKey& key) const noexcept {
///         auto seed = core::hash_range(key.attachments.begin(), key.attachments.end());
///         core::hash_and_combine(seed, key.extent);
///         core::hash_and_combine(seed, key.render_pass);
///         return seed;
///     }
/// };
/// ```
template <typename T>
struct Hash : public hash_impl::BaseHash<
                  T,
                  (std::is_enum_v<T> || std::is_integral_v<T> || std::is_pointer_v<T>)> {
};

template <>
struct Hash<f32> {
    [[nodiscard]] usize operator()(const f32& v) const noexcept
    {
        const f32 mapped_value = v == 0.f ? 0.f : v;
        return hash_impl::mumrmur_hash2(&mapped_value, sizeof(f32));
    }
};

template <>
struct Hash<f64> {
    [[nodiscard]] usize operator()(const f64& v) const noexcept
    {
        const f64 mapped_value = v == 0.0 ? 0.0 : v;
        return hash_impl::mumrmur_hash2(&mapped_value, sizeof(f64));
    }
};

///
constexpr void hash_combine(usize& seed, usize v) noexcept
{
    hash_impl::hash_combine_impl(seed, v);
}

///
template <typename T>
constexpr void hash_and_combine(usize& seed, const T& v) noexcept
{
    Hash<T> hasher;
    return hash_impl::hash_combine_impl(seed, hasher(v));
}

///
template <typename It>
[[nodiscard]] constexpr usize hash_range(It first, It last) noexcept
{
    usize seed = 0;
    for (; first != last; ++first) {
        hash_and_combine(seed, *first);
    }
    return seed;
}

namespace concepts {

///
template <typename T>
concept Hashable = requires(T v)
{
    // clang-format off
    { core::Hash<T> {}(v) } -> std::convertible_to<usize>;
    // clang-format on
};

} // namespace concepts

} // namespace tundra::core
