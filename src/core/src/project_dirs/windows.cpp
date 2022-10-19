#include "project_dirs/windows.h"

#if TNDR_PLATFORM_WINDOWS
#include <Windows.h>

#include <KnownFolders.h>
#include <ShlObj.h>

namespace tundra::core {

[[nodiscard]] core::Option<fs::path> get_known_folder(REFKNOWNFOLDERID folder_id) noexcept
{
    PWSTR path_ptr;
    const HRESULT result = SHGetKnownFolderPath(folder_id, 0, nullptr, &path_ptr);
    if (result == S_OK) {
        const usize len = lstrlenW(path_ptr);
        const fs::path path = fs::path(path_ptr, path_ptr + len);
        CoTaskMemFree(path_ptr);
        return path;
    } else {
        CoTaskMemFree(path_ptr);
        return std::nullopt;
    }
}

core::Option<Dirs> get_project_dirs(
    [[maybe_unused]] const std::string& qualifier,
    const std::string& organization,
    const std::string& app_name) noexcept
{
    const fs::path project_path = fs::path(organization) / fs::path(app_name);
    const core::Option<fs::path> local_app_data = get_known_folder(FOLDERID_LocalAppData);
    const core::Option<fs::path> roaming_app_data = get_known_folder(
        FOLDERID_RoamingAppData);

    if (local_app_data && roaming_app_data) {
        const fs::path project_local_app_data = *local_app_data / project_path;
        const fs::path project_roaming_app_data = *roaming_app_data / project_path;

        return Dirs {
            .project_path = project_path,
            .config_dir = project_roaming_app_data / "config",
            .cache_dir = project_local_app_data / "cache",
            .data_dir = project_roaming_app_data / "data",
            .data_local_dir = project_local_app_data / "data",
        };
    } else {
        return std::nullopt;
    }
}

} // namespace tundra::core

#endif