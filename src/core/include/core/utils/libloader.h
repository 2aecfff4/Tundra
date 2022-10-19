#pragma once
#include "core/core_export.h"
#include "core/core.h"

namespace tundra::core {

///
class CORE_API LibLoader {
public:
    struct LibHandle;

public:
    [[nodiscard]] static LibLoader::LibHandle* load(const char* name) noexcept;
    static void unload(const LibLoader::LibHandle* const lib) noexcept;

    template <typename T>
    [[nodiscard]] static T load_symbol(
        const LibLoader::LibHandle* const lib, const char* symbol_name) noexcept
    {
        return reinterpret_cast<T>(LibLoader::load_symbol_impl(lib, symbol_name));
    }

private:
    [[nodiscard]] static void* load_symbol_impl(
        const LibLoader::LibHandle* const lib, const char* symbol_name) noexcept;
};

} // namespace tundra::core
