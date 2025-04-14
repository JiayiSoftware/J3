#pragma once
#include "common.hpp"

#include "resource/resource.hpp"
#include "system/render/buffer/d3d_buffer.hpp"

class vertex_shader {
public:
    void initialize(const winrt::com_ptr<ID3D11Device>& device, const resource& data);
    
    winrt::com_ptr<ID3D11VertexShader>& get();
    const char* data();
    size_t size();
    d3d_buffer& get_constant_buffer();

    template<std::derived_from<cbuffer> T>
    void set_constant_buffer(const winrt::com_ptr<ID3D11Device>& device) {
        constant_buffer.initialize<T>(device);
    }

private:
    winrt::com_ptr<ID3D11VertexShader> shader;
    const char* shader_data = nullptr;
    size_t shader_size = 0;
    d3d_buffer constant_buffer;
};

class pixel_shader {
public:
    void initialize(const winrt::com_ptr<ID3D11Device>& device, const resource& data);
    
    winrt::com_ptr<ID3D11PixelShader>& get();
    const char* data();
    size_t size();
    d3d_buffer& get_constant_buffer();

    template<std::derived_from<cbuffer> T>
    void set_constant_buffer(const winrt::com_ptr<ID3D11Device>& device) {
        constant_buffer.initialize<T>(device);
    }

private:
    winrt::com_ptr<ID3D11PixelShader> shader;
    const char* shader_data = nullptr;
    size_t shader_size = 0;
    d3d_buffer constant_buffer;
};
