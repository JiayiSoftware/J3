#include "renderer.hpp"

#include <WICTextureLoader.h>

#include "mesh/vertex.hpp"
#include "resource/resource.hpp"

LOAD_RESOURCE(obj_vertex_vs_cso)
LOAD_RESOURCE(obj_pixel_ps_cso)

LOAD_RESOURCE(resources_textures_peppa_pig_png)

void renderer::initialize(const HWND handle, const vector2 size, const bool hardware_accelerated) {
    this->window_handle = handle;
    this->window_size = size;
    
    create_device_and_swap_chain(hardware_accelerated);
    create_render_target();
    create_depth_stencil();
    set_viewport();
    create_rasterizer();
    create_sampler();
    setup_shaders();
    load_textures();

    initialize_scene();
}

void renderer::render_frame() {
    float background[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    this->device_context->ClearRenderTargetView(this->render_target_view.get(), background);
    this->device_context->ClearDepthStencilView(this->depth_stencil_view.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    this->device_context->IASetInputLayout(input_layout.get());
    this->device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    this->device_context->RSSetState(rasterizer_state.get());
    this->device_context->OMSetDepthStencilState(depth_stencil_state.get(), 0);

    auto sampler = sampler_state.get();
    this->device_context->PSSetSamplers(0, 1, &sampler);
    auto texture = a_texture.get();
    this->device_context->VSSetShader(vs.get().get(), nullptr, 0);
    this->device_context->PSSetShader(ps.get().get(), nullptr, 0);

    const UINT stride = sizeof(vertex);
    const UINT offset = 0;
    
    auto vb = vertex_buffer.get();
    this->device_context->PSSetShaderResources(0, 1, &texture);
    this->device_context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
    this->device_context->Draw(6, 0);

    this->swap_chain->Present(1, 0);
}

void renderer::create_device_and_swap_chain(const bool hardware_accelerated) {
    DXGI_SWAP_CHAIN_DESC swap_chain_desc = { };

    swap_chain_desc.BufferDesc.Width = this->window_size.x;
    swap_chain_desc.BufferDesc.Height = this->window_size.y;
    
    // probably get the refresh rate from the monitor
    swap_chain_desc.BufferDesc.RefreshRate.Numerator = 60;
    swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
    
    swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swap_chain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    swap_chain_desc.SampleDesc.Count = 1;
    swap_chain_desc.SampleDesc.Quality = 0;

    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.BufferCount = 1;
    swap_chain_desc.OutputWindow = this->window_handle;
    swap_chain_desc.Windowed = true;
    swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // discard last frame after presenting
    swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // allow resizing
    
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, // video adapter, pass null to use best available
        hardware_accelerated ? D3D_DRIVER_TYPE_HARDWARE : D3D_DRIVER_TYPE_WARP, // driver type, either hardware or WARP (software)
        nullptr, // software rasterizer module if driver type is software (WARP is different)
        0, // creation flags, 0 for now
        nullptr, // array of feature levels, null to use default
        0, // number of feature levels, DirectX knows how many is in the default array
        D3D11_SDK_VERSION, // SDK version
        &swap_chain_desc, // swap chain description
        swap_chain.put(), // swap chain output
        device.put(), // device output
        nullptr, // feature level output, not needed
        device_context.put() // device context output
    );

    if (FAILED(hr)) {
        // handle error
    }
}

void renderer::create_render_target() {
    winrt::com_ptr<ID3D11Texture2D> back_buffer;
    HRESULT hr = this->swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), back_buffer.put_void());
    if (FAILED(hr)) {
        // handle error
    }
    
    hr = this->device->CreateRenderTargetView(back_buffer.get(), nullptr, this->render_target_view.put());
    if (FAILED(hr)) {
        // handle error
    }
}

void renderer::create_depth_stencil() {
    CD3D11_TEXTURE2D_DESC desc(DXGI_FORMAT_D24_UNORM_S8_UINT, this->window_size.x, this->window_size.y);
    desc.MipLevels = 1;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    HRESULT hr = this->device->CreateTexture2D(&desc, nullptr, this->depth_stencil_buffer.put());
    if (FAILED(hr)) {
        // handle error
    }

    hr = this->device->CreateDepthStencilView(this->depth_stencil_buffer.get(), nullptr, this->depth_stencil_view.put());
    if (FAILED(hr)) {
        // handle error
    }

    // set render target here instead of in create_render_target
    ID3D11RenderTargetView* render_target_view = this->render_target_view.get();
    this->device_context->OMSetRenderTargets(1, &render_target_view, this->depth_stencil_view.get());

    CD3D11_DEPTH_STENCIL_DESC depth_stencil_desc(D3D11_DEFAULT);
    depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

    hr = this->device->CreateDepthStencilState(&depth_stencil_desc, this->depth_stencil_state.put());
    if (FAILED(hr)) {
        // handle error
    }
}

void renderer::set_viewport() {
    CD3D11_VIEWPORT viewport(0.0f, 0.0f, this->window_size.x, this->window_size.y);
    this->device_context->RSSetViewports(1, &viewport);
}

void renderer::create_rasterizer() {
    const CD3D11_RASTERIZER_DESC desc(D3D11_DEFAULT); // solid fill mode, cull back, front is clockwise
    HRESULT hr = this->device->CreateRasterizerState(&desc, this->rasterizer_state.put());
    if (FAILED(hr)) {
        // handle error
    }
}

void renderer::create_sampler() {
    CD3D11_SAMPLER_DESC desc(D3D11_DEFAULT);
    HRESULT hr = this->device->CreateSamplerState(&desc, this->sampler_state.put());
    if (FAILED(hr)) {
        // handle error
    }
}

void renderer::setup_shaders() {
    D3D11_INPUT_ELEMENT_DESC* input_layout = vertex::get_input_layout();

    const auto vertex_shader_data = GET_RESOURCE(obj_vertex_vs_cso);
    const auto pixel_shader_data = GET_RESOURCE(obj_pixel_ps_cso);

    vs.initialize(device, vertex_shader_data);
    ps.initialize(device, pixel_shader_data);

    HRESULT hr = this->device->CreateInputLayout(
        input_layout,
        3,
        vs.data(),
        vs.size(), // shader bytecode size
        this->input_layout.put()
    );

    if (FAILED(hr)) {
        // handle error
    }
}

void renderer::load_textures() {
    resource tex = GET_RESOURCE(resources_textures_peppa_pig_png);
    
    HRESULT hr = DirectX::CreateWICTextureFromMemory(
        this->device.get(),
        reinterpret_cast<const uint8_t*>(tex.data()),
        tex.size(),
        nullptr,
        this->a_texture.put()
    );

    if (FAILED(hr)) {
        std::print("lol");
    }
}

void renderer::initialize_scene() {
    vertex v[] = {
        // tri 1
        { { -0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
        { { -0.5f, 0.5f, 1.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
        { { 0.5f, 0.5f, 1.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } },

        // tri 2
        { { -0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
        { { 0.5f, 0.5f, 1.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } },
        { { 0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } }
    };

    CD3D11_BUFFER_DESC desc(sizeof(vertex) * ARRAYSIZE(v), D3D11_BIND_VERTEX_BUFFER);
    D3D11_SUBRESOURCE_DATA data = { };
    data.pSysMem = v;

    HRESULT hr = this->device->CreateBuffer(&desc, &data, vertex_buffer.put());
    if (FAILED(hr)) {
        // handle error
    }
}
