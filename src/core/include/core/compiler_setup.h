#pragma once
#include "core/build.h"

// clang-format off

#if TNDR_COMPILER_MSVC
// #pragma warning(disable: 4251)  // 'type' needs to have dll-interface to be used by clients of 'type' https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-1-c4251
// #pragma warning(disable: 4530)  // C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
// #pragma warning(disable: 4324)  // Structure was padded due to alignment specifier
// #pragma warning(disable: 26812) // The enum type "x" is unscoped. Prefer "enum class" over "enum"(annoying with a lot of third-party code) 
#endif // COMPILER_MSVC

// clang-format on
