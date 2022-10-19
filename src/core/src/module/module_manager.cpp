#include "core/module/module_manager.h"
#include "core/logger.h"
#include "core/module/module.h"
#include "core/std/assert.h"
#include "core/std/containers/hash_map.h"
#include "core/std/containers/string.h"
#include "core/std/panic.h"
#include "core/std/sync/lock.h"
#include "core/std/sync/rw_lock.h"
#include "core/utils/libloader.h"

#if TNDR_PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

namespace tundra::core {

///
class ModuleManagerImpl final : public ModuleManager {
private:
    struct ModuleInfo {
        String name;
        String path;
        LibLoader::LibHandle* handle = nullptr;
        IModule* module_ptr = nullptr;
    };

    using InitializeModuleFn = IModule* (*)();

private:
    RwLock<HashMap<String, ModuleInfo>> m_modules;
#if TNDR_MONOLITHIC_BUILD
    Lock<HashMap<String, InitializeModuleFn>> m_registered_modules;
#endif

public:
    virtual ~ModuleManagerImpl() noexcept
    {
        this->unload_all_modules();
    }

private:
    virtual bool load_module(const char* name) noexcept override
    {
        auto modules = m_modules.write();
        if (auto it = modules->find(name); it != modules->end()) {
            tndr_warn("Module `{}` is already loaded!", name);
            return false;
        }

#if !TNDR_MONOLITHIC_BUILD

#if TNDR_PLATFORM_WINDOWS
        core::String new_name(name);
        new_name += ".dll";
#else
        core::String new_name(name);
        new_name += ".so";
#endif
        LibLoader::LibHandle* const module_handle = LibLoader::load(new_name.c_str());
        if (module_handle == nullptr) {
            return false;
        } else {
            const ModuleManagerImpl::InitializeModuleFn fn =
                LibLoader::load_symbol<ModuleManagerImpl::InitializeModuleFn>(
                    module_handle, "initialize_module");
            if (fn == nullptr) {
                LibLoader::unload(module_handle);
                tndr_error("Module `{}` couldn't be loaded!", name);
                return false;
            } else {
                core::IModule* const module_ptr = fn();

                if (module_ptr == nullptr) {
                    tndr_error("Module `{}` couldn't be loaded!", name);
                    LibLoader::unload(module_handle);
                    return false;
                } else {
                    modules->insert({
                        name,
                        ModuleInfo {
                            .name = name,
                            .path = new_name,
                            .handle = module_handle,
                            .module_ptr = module_ptr,
                        },
                    });

                    tndr_info("Module `{}` has been successfully loaded.", name);
                    return true;
                }
            }
        }
#else
        auto registered_modules = m_registered_modules.lock();
        const auto it = registered_modules->find(name);
        if (it == registered_modules->end()) {
            tndr_warn("Module `{}` does not exist!", name);
            return false;
        }

        auto& fn = it->second;
        core::IModule* const module_ptr = fn();

        if (module_ptr == nullptr) {
            tndr_warn("Module `{}` couldn't be loaded!", name);
            return false;
        } else {
            modules->insert({
                name,
                ModuleInfo {
                    .name = name,
                    .path = name,
                    .handle = nullptr,
                    .module_ptr = module_ptr,
                },
            });

            tndr_info("Module `{}` has been successfully loaded.", name);
            return true;
        }
#endif
    }

    virtual void unload_module(const char* name) noexcept override
    {
        auto modules = m_modules.write();
        if (auto it = modules->find(name); it != modules->end()) {
            const ModuleManagerImpl::ModuleInfo& module_info = it->second;
            tndr_assert(module_info.module_ptr != nullptr, "");

            delete module_info.module_ptr;

            if (module_info.handle != nullptr) {
                LibLoader::unload(module_info.handle);
            }

            modules->erase(it);
            tndr_info("Module `{}` has been successfully unloaded.", name);
        } else {
            tndr_error("Module `{}` is not loaded.", name);
        }
    }

    [[nodiscard]] virtual IModule* get_module(const char* name) noexcept override
    {
        auto modules = m_modules.read();
        if (auto it = modules->find(name); it != modules->end()) {
            const ModuleManagerImpl::ModuleInfo& module_info = it->second;
            return module_info.module_ptr;
        } else {
            return nullptr;
        }
    }

    [[nodiscard]] virtual bool is_module_loaded(const char* name) noexcept override
    {
        auto modules = m_modules.read();
        const auto it = modules->find(name);
        return it != modules->end();
    }

    virtual void unload_all_modules() noexcept override
    {
        auto modules = m_modules.write();
        for (const auto& [name, module_info] : *modules) {
            tndr_assert(module_info.module_ptr != nullptr, "");

            delete module_info.module_ptr;

            if (module_info.handle != nullptr) {
                LibLoader::unload(module_info.handle);
            }

            tndr_info("Module `{}` has been successfully unloaded.", module_info.name);
        }

        modules->clear();
    }

#if TNDR_MONOLITHIC_BUILD
    virtual void register_module(const char* name, IModule* (*func)()) noexcept override
    {
        tndr_assert(func != nullptr, "");
        auto registered_modules = m_registered_modules.lock();

#if TNDR_BUILD_DEBUG
        if (auto it = registered_modules->find(name); it != registered_modules->end()) {
            tndr_panic("Module `{}` is already registered.", name);
        }
#endif

        registered_modules->insert({
            name,
            func,
        });
    }
#endif
};

ModuleManager& ModuleManager::get() noexcept
{
    static ModuleManagerImpl s_singleton;
    return s_singleton;
}

} // namespace tundra::core
