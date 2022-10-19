#pragma once
#include "core/core_export.h"
#include "core/core.h"
#include "core/std/containers/string.h"
#include <filesystem>

namespace tundra::core {

///
class CORE_API ProjectDirs {
private:
    std::filesystem::path m_project_path;
    std::filesystem::path m_config_dir;
    std::filesystem::path m_cache_dir;
    std::filesystem::path m_data_dir;
    std::filesystem::path m_data_local_dir;

public:
    ProjectDirs(
        const std::string& qualifier,
        const std::string& organization,
        const std::string& app_name) noexcept;

public:
    [[nodiscard]] const std::filesystem::path& get_project_path() const noexcept;
    [[nodiscard]] const std::filesystem::path& get_config_dir() const noexcept;
    [[nodiscard]] const std::filesystem::path& get_cache_dir() const noexcept;
    [[nodiscard]] const std::filesystem::path& get_data_dir() const noexcept;
    [[nodiscard]] const std::filesystem::path& get_data_local_dir() const noexcept;
};

} // namespace tundra::core
