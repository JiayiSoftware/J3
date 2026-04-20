#include "minecraft.hpp"

#include "framework/core/special_folder.hpp"

#include <winrt/windows.foundation.collections.h>
#include <winrt/windows.storage.h>

minecraft_version minecraft::version() {
    if (auto info = this->get_app_info()) {
        auto version = info->Package().Id().Version();
        
        uint16_t minor = version.Minor;
        
        // weird
        std::string build = std::to_string(version.Build);
        if (build.length() != 1) {
            build = build.substr(0, build.length() - 2);
        }
        
        // major will always be 1 (pre GDK, 26.0)
        return {
            minor >= 26
            ? std::format("{}.{}", minor, build)
            : std::format("1.{}.{}", minor, build)
        };
    }
    
    return { };
}

std::filesystem::path minecraft::install_path() {
    if (auto info = this->get_app_info()) {
        return { info->Package().InstalledLocation().Path().c_str() };
    }
    
    spdlog::warn("Call to install_path failed; the game is probably not installed");
    return { };
}

std::filesystem::path minecraft::data_path() {
    minecraft_version version = this->version();
    if (version.empty()) return { };
    
    // data path changed since GDK (26.0)
    return version[0] == '1' 
        ? special_folder::get(FOLDERID_LocalAppData) / "Packages" / FAMILY_NAME
        : special_folder::get(FOLDERID_LocalAppData) / "Minecraft Bedrock";
}

bool minecraft::is_store_managed() {
    if (auto info = this->get_app_info()) {
        // installed in development mode == not from the store
        return !info->Package().IsDevelopmentMode();
    }
    
    // most likely not installed
    spdlog::warn("Call to is_store_managed failed; the game is probably not installed");
    return false;
}

std::optional<AppInfo> minecraft::get_app_info() {
    auto info_vec = AppDiagnosticInfo::RequestInfoForPackageAsync(FAMILY_NAME).get();
    
    return info_vec.Size() != 0 ? std::optional{ info_vec.GetAt(0).AppInfo() } : std::nullopt;
}