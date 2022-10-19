#include "core/utils/libloader.h"
#include "core/std/assert.h"

#if TNDR_PLATFORM_WINDOWS
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

namespace tundra::core {

LibLoader::LibHandle* LibLoader::load(const char* name) noexcept
{
#if TNDR_PLATFORM_WINDOWS
    return reinterpret_cast<LibLoader::LibHandle*>(LoadLibraryA(name));
#else
    return reinterpret_cast<LibLoader::LibHandle*>(dlopen(name, RTLD_NOW | RTLD_LOCAL));
#endif
}

void LibLoader::unload(const LibLoader::LibHandle* const lib) noexcept
{
    tndr_assert(lib != nullptr, "`lib` is nullptr");

#if TNDR_PLATFORM_WINDOWS
    FreeLibrary(reinterpret_cast<HMODULE>(const_cast<LibLoader::LibHandle*>(lib)));
#else
    return dlclose(reinterpret_cast<void*>(const_cast<LibLoader::LibHandle*>(lib)));
#endif
}

void* LibLoader::load_symbol_impl(
    const LibLoader::LibHandle* const lib, const char* symbol_name) noexcept
{
    tndr_assert(lib != nullptr, "`lib` is nullptr");
    tndr_assert(symbol_name != nullptr, "`symbol_name` is nullptr");

#if TNDR_PLATFORM_WINDOWS
    return GetProcAddress(
        reinterpret_cast<HMODULE>(const_cast<LibLoader::LibHandle*>(lib)), symbol_name);
#else
    return dlsym(
        reinterpret_cast<void*>(const_cast<LibLoader::LibHandle*>(lib)), symbol_name);
#endif
}

} // namespace tundra::core
