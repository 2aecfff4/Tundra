#pragma once
#include "core/core.h"
#if TNDR_PLATFORM_WINDOWS
#include "core/std/option.h"
#include <filesystem>

namespace fs = std::filesystem;

namespace tundra::core {

struct Dirs {
    fs::path project_path;
    fs::path config_dir;
    fs::path cache_dir;
    fs::path data_dir;
    fs::path data_local_dir;
};

[[nodiscard]] core::Option<Dirs> get_project_dirs(
    const std::string& qualifier,
    const std::string& organization,
    const std::string& app_name) noexcept;

} // namespace tundra::core

#endif
