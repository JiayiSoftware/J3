#include "shader.hpp"

void vertex_shader::initialize(const winrt::com_ptr<ID3D11Device>& device, const resource& data) {
    this->shader_data = data.data();
    this->shader_size = data.size();
    
    HRESULT hr = device->CreateVertexShader(data.data(), data.size(), nullptr, shader.put());
    if (FAILED(hr)) {
        // handle error
    }
}

winrt::com_ptr<ID3D11VertexShader>& vertex_shader::get() {
    return shader;
}

const char* vertex_shader::data() {
    return shader_data;
}

size_t vertex_shader::size() {
    return shader_size;
}

void pixel_shader::initialize(const winrt::com_ptr<ID3D11Device>& device, const resource& data) {
    this->shader_data = data.data();
    this->shader_size = data.size();
    
    HRESULT hr = device->CreatePixelShader(data.data(), data.size(), nullptr, shader.put());
    if (FAILED(hr)) {
        // handle error
    }
}

winrt::com_ptr<ID3D11PixelShader>& pixel_shader::get() {
    return shader;
}

const char* pixel_shader::data() {
    return shader_data;
}

size_t pixel_shader::size() {
    return shader_size;
}
