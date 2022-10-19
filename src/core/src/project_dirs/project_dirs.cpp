#include "core/project_dirs.h"
#include "core/std/panic.h"
#include "project_dirs/windows.h"

namespace tundra::core {

ProjectDirs::ProjectDirs(
    const std::string& qualifier,
    const std::string& organization,
    const std::string& app_name) noexcept
{
#if TNDR_PLATFORM_WINDOWS
    const core::Option<core::Dirs> dirs = get_project_dirs(
        qualifier, organization, app_name);
    if (dirs) {
        m_project_path = dirs->project_path;
        m_config_dir = dirs->config_dir;
        m_cache_dir = dirs->cache_dir;
        m_data_dir = dirs->data_dir;
        m_data_local_dir = dirs->data_local_dir;
    } else {
        panic("`get_project_dirs` returned `std::nullopt`!");
    }
#else
#error Unsupported platform!
#endif
}

const std::filesystem::path& ProjectDirs::get_project_path() const noexcept
{
    return m_project_path;
}

const std::filesystem::path& ProjectDirs::get_config_dir() const noexcept
{
    return m_config_dir;
}

const std::filesystem::path& ProjectDirs::get_cache_dir() const noexcept
{
    return m_cache_dir;
}

const std::filesystem::path& ProjectDirs::get_data_dir() const noexcept
{
    return m_data_dir;
}

const std::filesystem::path& ProjectDirs::get_data_local_dir() const noexcept
{
    return m_data_local_dir;
}

} // namespace tundra::core
