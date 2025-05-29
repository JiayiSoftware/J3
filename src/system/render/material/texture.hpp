#pragma once
#include "common.hpp"
#include "resource/resource.hpp"

struct texture {
    std::wstring path;
    // or
    resource res;
    
    vector2 size = { 0, 0 };

    // renderer ensures that this is loaded
    winrt::com_ptr<ID3D11ShaderResourceView> texture_view = nullptr;

    texture(const std::wstring& path) : path(path), res(resource::empty()) { }
    texture(const resource& resource) : res(resource) { }
};
