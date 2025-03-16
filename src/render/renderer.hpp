#pragma once
#include "common.hpp"

#include "buffer/d3d_buffer.hpp"
#include "shader/shader.hpp"

class adapter_data {
public:
    winrt::com_ptr<IDXGIAdapter> adapter;
    DXGI_ADAPTER_DESC description;

    explicit adapter_data(const winrt::com_ptr<IDXGIAdapter>& adapter) : adapter(adapter) {
        this->adapter->GetDesc(&description);
    }
};

class renderer {
public:
    void initialize(HWND handle, vector2 size, bool hardware_accelerated = true);
    void render_frame();

private:
    HWND window_handle = nullptr;
    vector2 window_size = { 0, 0 };
    
    winrt::com_ptr<ID3D11Device> device;
    winrt::com_ptr<ID3D11DeviceContext> device_context;
    winrt::com_ptr<IDXGISwapChain> swap_chain;
    winrt::com_ptr<ID3D11RenderTargetView> render_target_view;

    winrt::com_ptr<ID3D11InputLayout> input_layout;
    vertex_shader vs;
    pixel_shader ps;

    d3d_buffer vertex_buffer;
    d3d_buffer index_buffer;
    d3d_buffer constant_buffer;
    
    winrt::com_ptr<ID3D11RasterizerState> rasterizer_state;

    winrt::com_ptr<ID3D11DepthStencilView> depth_stencil_view;
    winrt::com_ptr<ID3D11DepthStencilState> depth_stencil_state;
    winrt::com_ptr<ID3D11Texture2D> depth_stencil_buffer;

    winrt::com_ptr<ID3D11SamplerState> sampler_state;
    winrt::com_ptr<ID3D11ShaderResourceView> a_texture;
    
    void create_device_and_swap_chain(bool hardware_accelerated);
    void create_render_target();
    void create_depth_stencil();
    void set_viewport();
    void create_rasterizer();
    void create_sampler();
    void setup_shaders();
    void load_textures();
    void create_constant_buffers();
    void initialize_scene();
};
