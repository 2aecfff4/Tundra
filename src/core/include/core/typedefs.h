#pragma once
#include "core/build.h"
#include <cstdint>

// Unsigned base types.
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

#if TNDR_PLATFORM_64_BIT
using usize = u64;
#elif TNDR_PLATFORM_32_BIT
using usize = u32;
#endif

// Signed base types.
using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

#if TNDR_PLATFORM_64_BIT
using isize = i64;
#elif TNDR_PLATFORM_32_BIT
using isize = i32;
#endif

// floats
using f32 = float;
using f64 = double;
