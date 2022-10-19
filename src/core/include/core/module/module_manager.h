#pragma once
#include "core/core_export.h"
#include "core/core.h"

namespace tundra::core {

class IModule;

/// Module manager is thread safe.
class CORE_API ModuleManager {
protected:
    ModuleManager() noexcept = default;
    virtual ~ModuleManager() noexcept = default;

public:
    ModuleManager(ModuleManager&&) noexcept = delete;
    ModuleManager& operator=(ModuleManager&&) noexcept = delete;
    ModuleManager(const ModuleManager&) noexcept = delete;
    ModuleManager& operator=(const ModuleManager&) noexcept = delete;

public:
    [[nodiscard]] static ModuleManager& get() noexcept;

public:
    virtual bool load_module(const char* name) noexcept = 0;
    virtual void unload_module(const char* name) noexcept = 0;

    /// Get a module by name.
    /// Returns a valid pointer if module is loaded, otherwise returns nullptr.
    [[nodiscard]] virtual IModule* get_module(const char* name) noexcept = 0;

    template <typename T>
    [[nodiscard]] T* get_module(const char* name) noexcept
    {
        IModule* const ptr = this->get_module(name);
        return static_cast<T* const>(ptr);
    }

    [[nodiscard]] virtual bool is_module_loaded(const char* name) noexcept = 0;

    virtual void unload_all_modules() noexcept = 0;

#if TNDR_MONOLITHIC_BUILD

private:
    virtual void register_module(const char* name, IModule* (*func)()) noexcept = 0;

private:
    template <typename T>
    friend struct StaticModuleRegistrar;
#endif // TNDR_MONOLITHIC_BUILD
};

} // namespace tundra::core
