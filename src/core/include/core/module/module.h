#pragma once
#include "core/core_export.h"
#include "core/core.h"
#include <new>
#if TNDR_MONOLITHIC_BUILD
#include "core/module/module_manager.h"
#endif

namespace tundra::core {

///
class CORE_API IModule {
public:
    virtual ~IModule() noexcept = default;
    virtual void load_module()
    {
    }

    virtual void unload_module()
    {
    }

    [[nodiscard]] virtual const char* get_name() const noexcept
    {
        return "";
    }

    [[nodiscard]] virtual bool supports_dynamic_reloading() const noexcept
    {
        return false;
    }
};

///
class CORE_API DefaultModuleImplementation : public IModule {
    [[nodiscard]] virtual const char* get_name() const noexcept override
    {
        return "DefaultModuleImplementation";
    }
};

} // namespace tundra::core

#if TNDR_MONOLITHIC_BUILD

///
template <typename T>
struct StaticModuleRegistrar {
    StaticModuleRegistrar(const char* name) noexcept
    {
        // In monolithic build we are not instantiating module class.
        // Module class is instantiated only when a user wants to load module.
        ModuleManager::get().register_module(
            name, []() -> tundra::core::IModule* { return new T {}; });
    }
};
#endif // TNDR_MONOLITHIC_BUILD

#if TNDR_COMPILER_MSVC
#define MSVC_ANNOTATION __pragma(warning(suppress : 28251))
#else // TNDR_COMPILER_MSVC
#define MSVC_ANNOTATION
#endif

#if TNDR_ENABLE_THREAD_SANITIZER || TNDR_ENABLE_ADDRESS_SANITIZER
#define TNDR_INTERNAL_IMPLEMENT_NEW_OPERATORS
#else
#define TNDR_INTERNAL_IMPLEMENT_NEW_OPERATORS
#endif

#if TNDR_MONOLITHIC_BUILD

///
#define TNDR_IMPLEMENT_MODULE(clazz, name)                                               \
    TNDR_INTERNAL_IMPLEMENT_NEW_OPERATORS                                                \
    [[maybe_unused]] static StaticModuleRegistrar<clazz> TNDR_APPEND(                    \
        StaticModuleRegistrar, name) { name };

#else

///
#define TNDR_IMPLEMENT_MODULE(clazz, name)                                               \
    TNDR_INTERNAL_IMPLEMENT_NEW_OPERATORS                                                \
    extern "C" TNDR_DLL_EXPORT tundra::core::IModule* initialize_module()                \
    {                                                                                    \
        return new clazz {};                                                             \
    }                                                                                    \
    extern "C" TNDR_DLL_EXPORT const char* get_module_name()                             \
    {                                                                                    \
        return name;                                                                     \
    }

#endif
