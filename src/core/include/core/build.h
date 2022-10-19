#pragma once

//////////////////////////////////////////////////////////////////////////
// Platforms

#if defined(WIN32) || defined(_WIN32)
#define TNDR_PLATFORM_WINDOWS 1
#else
#define TNDR_PLATFORM_WINDOWS 0
#endif

#if (defined(linux) || defined(__linux) || defined(__linux__))
#define TNDR_PLATFORM_LINUX 1
#else
#define TNDR_PLATFORM_LINUX 0
#endif

#if defined(ANDROID) || defined(__ANDROID__)
#define TNDR_PLATFORM_ANDROID 1
#else
#define TNDR_PLATFORM_ANDROID 0
#endif

#if defined(__APPLE__)
#define TNDR_PLATFORM_MAC 1
#else
#define TNDR_PLATFORM_MAC 0
#endif

#if defined(IOS)
#define TNDR_PLATFORM_IOS 1
#else
#define TNDR_PLATFORM_IOS 0
#endif

#if defined(i386) || defined(__i386) || defined(__i386__) || defined(_M_IX86)
#define TNDR_PLATFORM_X86 1
#else
#define TNDR_PLATFORM_X86 0
#endif

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) ||                     \
    defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64)
#define TNDR_PLATFORM_X64 1
#else
#define TNDR_PLATFORM_X64 0
#endif

#if TNDR_PLATFORM_X64 || TNDR_PLATFORM_X86
#define TNDR_STRONG_MEMORY_MODEL 1
#else // TNDR_PLATFORM_X64 || TNDR_PLATFORM_X86
#define TNDR_STRONG_MEMORY_MODEL 0
#endif

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) ||                     \
    defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64) || defined(__ia64) ||      \
    defined(_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
#define TNDR_PLATFORM_64_BIT 1
#define TNDR_PLATFORM_32_BIT 0
#else
#define TNDR_PLATFORM_64_BIT 0
#define TNDR_PLATFORM_32_BIT 1
#endif

//////////////////////////////////////////////////////////////////////////
// Compilers.

#if defined(_MSC_VER) && !defined(__clang__) && !(defined(__GNUC__) || defined(__GNUG__))
#define TNDR_COMPILER_MSVC 1
#else
#define TNDR_COMPILER_MSVC 0
#endif

#if (defined(__GNUC__) || defined(__GNUG__)) && !defined(__clang__)
#define TNDR_COMPILER_GCC 1
#else
#define TNDR_COMPILER_GCC 0
#endif

#if defined(__clang__)
#define TNDR_COMPILER_CLANG 1
#else
#define TNDR_COMPILER_CLANG 0
#endif

//////////////////////////////////////////////////////////////////////////
// Endianness

#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) &&                       \
    (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define TNDR_LITTLE_ENDIAN 1
#elif defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) &&                        \
    (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define TNDR_LITTLE_ENDIAN 0
#elif defined(__BYTE_ORDER__) && defined(__ORDER_PDP_ENDIAN__) &&                        \
    (__BYTE_ORDER__ == __ORDER_PDP_ENDIAN__)
#error PDP endianness is not supported!
#elif defined(__LITTLE_ENDIAN__)
#define TNDR_LITTLE_ENDIAN 1
#elif defined(__BIG_ENDIAN__)
#define TNDR_LITTLE_ENDIAN 0
#elif defined(_MSC_VER) || defined(__i386__) || defined(__x86_64__)
#define TNDR_LITTLE_ENDIAN 1
#else
#error The engine could not determine the endianness of this platform.
#endif

//////////////////////////////////////////////////////////////////////////
//

#if defined(NDEBUG)
#define TNDR_BUILD_DEBUG 0
#else
#define TNDR_BUILD_DEBUG 1
#endif

/// #TODO
#define TNDR_BUILD_SHIPPING 0

/// #TODO
#define TNDR_WITH_DEVELOPMENT_TOOLS 1

/// #TODO
#define TNDR_WITH_METADATA 1

#if TUNDRA_MONOLITHIC_BUILD
#define TNDR_MONOLITHIC_BUILD 1
#else
#define TNDR_MONOLITHIC_BUILD 0
#endif

//////////////////////////////////////////////////////////////////////////
//

#if TNDR_MONOLITHIC_BUILD
#define TNDR_DLL_EXPORT
#define TNDR_DLL_IMPORT
#else
#if TNDR_PLATFORM_WINDOWS
#define TNDR_DLL_EXPORT __declspec(dllexport)
#define TNDR_DLL_IMPORT __declspec(dllimport)
#elif TNDR_PLATFORM_LINUX || TNDR_PLATFORM_MAC || TNDR_PLATFORM_ANDROID ||               \
    TNDR_PLATFORM_IOS
#define TNDR_DLL_EXPORT __attribute__((visibility("default")))
#define TNDR_DLL_IMPORT __attribute__((visibility("default")))
#endif
#endif
