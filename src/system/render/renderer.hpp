#pragma once
#include "common.hpp"

#include "buffer/d3d_buffer.hpp"
#include "material/texture.hpp"
#include "mesh/mesh.hpp"
#include "shader/shader.hpp"

class adapter_data {
public:
    winrt::com_ptr<IDXGIAdapter> adapter;
    DXGI_ADAPTER_DESC description;

    explicit adapter_data(const winrt::com_ptr<IDXGIAdapter>& adapter) : adapter(adapter) {
        this->adapter->GetDesc(&description);
    }
};

struct renderer {
    renderer(HWND handle, vector2 size, bool hardware_accelerated = true);
    void initialize();
    void update(entt::registry& registry);
    void destroy();

    void set_background_color(vector4 col);

private:
    HWND window_handle = nullptr;
    vector2 window_size = { 0, 0 };
    bool hardware_accelerated = true; // by default
    std::array<float, 4> background_color;
    
    winrt::com_ptr<ID3D11Device> device;
    winrt::com_ptr<ID3D11DeviceContext> device_context;
    winrt::com_ptr<IDXGISwapChain> swap_chain;
    winrt::com_ptr<ID3D11RenderTargetView> render_target_view;

    winrt::com_ptr<ID3D11InputLayout> input_layout;

    d3d_buffer vertex_buffer;
    d3d_buffer index_buffer;
    
    winrt::com_ptr<ID3D11RasterizerState> rasterizer_state;
    winrt::com_ptr<ID3D11SamplerState> sampler_state;
    winrt::com_ptr<ID3D11BlendState> blend_state;
    
    void render_frame(entt::registry& registry);
    
    void create_device_and_swap_chain();
    void create_render_target();
    void set_viewport();
    void create_rasterizer();
    void create_sampler();
    void create_blend_state();
    void create_default_resources();

    void ensure_texture_loaded(const std::shared_ptr<texture>& tex);
    void ensure_mesh_buffers_created(const std::shared_ptr<mesh>& m);
};
