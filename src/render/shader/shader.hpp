#pragma once
#include "common.hpp"

#include "resource/resource.hpp"

class vertex_shader {
public:
    void initialize(const winrt::com_ptr<ID3D11Device>& device, const resource& data);
    
    winrt::com_ptr<ID3D11VertexShader>& get();
    const char* data();
    size_t size();

private:
    winrt::com_ptr<ID3D11VertexShader> shader;
    const char* shader_data = nullptr;
    size_t shader_size = 0;
};

class pixel_shader {
public:
    void initialize(const winrt::com_ptr<ID3D11Device>& device, const resource& data);
    
    winrt::com_ptr<ID3D11PixelShader>& get();
    const char* data();
    size_t size();

private:
    winrt::com_ptr<ID3D11PixelShader> shader;
    const char* shader_data = nullptr;
    size_t shader_size = 0;
};
