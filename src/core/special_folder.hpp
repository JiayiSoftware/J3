#pragma once
#include "common.hpp"

namespace special_folder {
    inline std::filesystem::path get(REFKNOWNFOLDERID id) {
        wchar_t* path;

        // hate this function .
        HRESULT hr = SHGetKnownFolderPath(id, 0, nullptr, &path);
        if (FAILED(hr)) return std::filesystem::path(L"");

        std::wstring str(path);
        return std::filesystem::path(str);
    }
}
