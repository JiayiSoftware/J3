#pragma once
#include "common.hpp"
#include "j3/version/minecraft_version.hpp"

#include <winrt/windows.applicationmodel.h>
#include <winrt/windows.system.h>

using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::System;

class minecraft {
public:
    static constexpr std::wstring_view FAMILY_NAME{ L"Microsoft.MinecraftUWP_8wekyb3d8bbwe" };
    
    minecraft_version version();
    std::filesystem::path install_path();
    std::filesystem::path data_path();
    bool is_store_managed();
    
private:
    std::optional<AppInfo> get_app_info();
};
