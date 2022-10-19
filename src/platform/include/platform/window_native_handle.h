#pragma once
#include "core/core.h"
#include "core/std/variant.h"

namespace tundra::platform {

#if TNDR_PLATFORM_WINDOWS

///
struct Win32Handle {
    void* hwnd = nullptr;
    void* hinstance = nullptr;
};

#elif TNDR_PLATFORM_LINUX

///
struct XlibHandle {
    unsigned long window = 0;
    void* display = nullptr;
};

///
struct XcbHandle {
    u32 window = 0;
    void* connection = nullptr;
};

///
struct WaylandHandle {
    void* surface = nullptr;
    void* display = nullptr;
};

#elif TNDR_PLATFORM_ANDROID

///
struct AndroidNdkHandle {
    void* a_native_window = nullptr;
};

#else
#error Unsupported platform!
#endif

///
using WindowNativeHandle = core::Variant<
#if TNDR_PLATFORM_WINDOWS
    Win32Handle
#elif TNDR_PLATFORM_LINUX
    XlibHandle,
    XcbHandle,
    WaylandHandle
#elif TNDR_PLATFORM_ANDROID
    AndroidNdkHandle
#endif
    >;

} // namespace tundra::platform
