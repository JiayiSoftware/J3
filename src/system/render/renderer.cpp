#include "renderer.hpp"

#include <WICTextureLoader.h>

#include "component/basic/drawable.hpp"
#include "component/basic/transform.hpp"
#include "mesh/vertex.hpp"
#include "resource/resource.hpp"

LOAD_RESOURCE(obj_vertex_vs_cso)
LOAD_RESOURCE(obj_pixel_ps_cso)

renderer::renderer(const HWND handle, const vector2 size, const bool hardware_accelerated) {
    this->window_handle = handle;
    this->window_size = size;
    this->hardware_accelerated = hardware_accelerated;
}

void renderer::initialize() {
    create_device_and_swap_chain();
    create_render_target();
    set_viewport();
    create_rasterizer();
    create_sampler();
    create_blend_state();
    create_default_resources();

    set_background_color({ 0.0f, 0.0f, 0.0f, 1.0f });
}

void renderer::update(entt::registry& registry) {
    // task list:
    // - check if resizing is needed
    // - add render_layer component
    // - sort entities by render_layer if layout is dirty
    // - render entities in order
    
    render_frame(registry);
}

void renderer::destroy() {
    //
}

void renderer::set_background_color(const vector4 col) {
    background_color[0] = col.x;
    background_color[1] = col.y;
    background_color[2] = col.z;
    background_color[3] = col.w;
}

void renderer::render_frame(entt::registry& registry) {
    this->device_context->ClearRenderTargetView(this->render_target_view.get(), background_color.data());

    this->device_context->IASetInputLayout(input_layout.get());
    this->device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    this->device_context->RSSetState(rasterizer_state.get());
    this->device_context->OMSetBlendState(this->blend_state.get(), nullptr, 0xFFFFFFFF);

    auto sampler = sampler_state.get();
    this->device_context->PSSetSamplers(0, 1, &sampler);

    // entities should be sorted at this point
    auto view = registry.view<drawable, transform>();
    for (auto entity : view) {
        // my rendering pipeline!!!
        
        auto& d = view.get<drawable>(entity);
        auto& t = view.get<transform>(entity);

        // 1: ensure all resources are loaded
        ensure_texture_loaded(d.tex);
        ensure_mesh_buffers_created(d.mesh);

        // 2: set shaders and resources
        auto& app = application::get();
        
        auto vs = d.vs != nullptr ? d.vs : app.resources.get<vertex_shader>("default");
        auto ps = d.ps != nullptr ? d.ps : app.resources.get<pixel_shader>("default");
        this->device_context->VSSetShader(vs->get().get(), nullptr, 0);
        this->device_context->PSSetShader(ps->get().get(), nullptr, 0);

        if (d.tex != nullptr) {
            auto srv = d.tex->texture_view.get();
            this->device_context->PSSetShaderResources(0, 1, &srv);
        }
        
        // 3: update matrices and set constant buffers
        cb_vertex cbv = d.vs_cbuffer; // copies existing values in case they were set somewhere else
        cbv.mat = t.get_matrix();

        // this could go somewhere else if i planned to do more with it
        static matrix projection = DirectX::XMMatrixOrthographicOffCenterLH(
            0.0f, this->window_size.x,
            this->window_size.y, 0.0f,
            0.0f, 1.0f
        );

        cbv.mat *= projection;
        cbv.mat = DirectX::XMMatrixTranspose(cbv.mat);

        auto& vertex_constant_buffer = vs->get_constant_buffer();
        vertex_constant_buffer.edit(device_context, &cbv, sizeof(cbv)); // is reusing the same buffer okay?

        cb_pixel cbp = d.ps_cbuffer;
        cbp.has_color = d.tex != nullptr ? 0.0f : 1.0f;

        auto& pixel_constant_buffer = ps->get_constant_buffer();
        pixel_constant_buffer.edit(device_context, &cbp, sizeof(cbp));
        
        auto cb_ptr = vertex_constant_buffer.get().get();
        auto cbp_ptr = pixel_constant_buffer.get().get();
        this->device_context->VSSetConstantBuffers(0, 1, &cb_ptr);
        this->device_context->PSSetConstantBuffers(0, 1, &cbp_ptr);

        // 4: set vertex and index buffers
        const UINT offset = 0;
        auto vb = d.mesh->vertex_buffer->get().get();
        this->device_context->IASetVertexBuffers(0, 1, &vb, d.mesh->vertex_buffer->stride_ptr(), &offset);
        this->device_context->IASetIndexBuffer(d.mesh->index_buffer->get().get(), DXGI_FORMAT_R32_UINT, 0);

        // 5: draw
        this->device_context->DrawIndexed(d.mesh->indices.size(), 0, 0);
    }

    HRESULT hr = this->swap_chain->Present(1, 0);
    if (FAILED(hr)) {
        // log error
    }
}

void renderer::create_device_and_swap_chain() {
    DXGI_SWAP_CHAIN_DESC swap_chain_desc = { };

    swap_chain_desc.BufferDesc.Width = static_cast<UINT>(this->window_size.x);
    swap_chain_desc.BufferDesc.Height = static_cast<UINT>(this->window_size.y);
    
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
        this->hardware_accelerated ? D3D_DRIVER_TYPE_HARDWARE : D3D_DRIVER_TYPE_WARP, // driver type, either hardware or WARP (software)
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

    auto rtv = this->render_target_view.get();
    this->device_context->OMSetRenderTargets(1, &rtv, nullptr);
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

void renderer::create_blend_state() {
    D3D11_RENDER_TARGET_BLEND_DESC rt_blend_desc = {};

    rt_blend_desc.BlendEnable = true;
    rt_blend_desc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
    rt_blend_desc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    rt_blend_desc.BlendOp = D3D11_BLEND_OP_ADD;
    rt_blend_desc.SrcBlendAlpha = D3D11_BLEND_ONE;
    rt_blend_desc.DestBlendAlpha = D3D11_BLEND_ZERO;
    rt_blend_desc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    rt_blend_desc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    
    D3D11_BLEND_DESC blend_desc = { };
    blend_desc.RenderTarget[0] = rt_blend_desc;

    HRESULT hr = this->device->CreateBlendState(&blend_desc, this->blend_state.put());
    if (FAILED(hr)) {
        // handle error
    }
}

void renderer::create_default_resources() {
    auto& app = application::get();
    
    const auto vertex_shader_data = GET_RESOURCE(obj_vertex_vs_cso);
    const auto pixel_shader_data = GET_RESOURCE(obj_pixel_ps_cso);
    
    auto default_vs = app.resources.add<vertex_shader>("default");
    auto default_ps = app.resources.add<pixel_shader>("default");
    default_vs->initialize(this->device, vertex_shader_data);
    default_vs->set_constant_buffer<cb_vertex>(this->device);
    default_ps->initialize(this->device, pixel_shader_data);
    default_ps->set_constant_buffer<cb_pixel>(this->device);

    // create input layout here because it's the same for all vertex shaders
    D3D11_INPUT_ELEMENT_DESC* input_layout = vertex::get_input_layout();
    HRESULT hr = this->device->CreateInputLayout(
        input_layout,
        3,
        default_vs->data(),
        default_vs->size(), // shader bytecode size
        this->input_layout.put()
    );

    if (FAILED(hr)) {
        // handle error
    }
}

void renderer::ensure_texture_loaded(const std::shared_ptr<texture>& tex) {
    if (tex == nullptr || tex->texture_view != nullptr) {
        return;
    }
    
    HRESULT hr;
    winrt::com_ptr<ID3D11Resource> texture_resource;
    
    if (tex->path != L"") {
        // load texture from file
        hr = DirectX::CreateWICTextureFromFile(
            this->device.get(),
            tex->path.c_str(),
            texture_resource.put(),
            tex->texture_view.put()
        );
    } else {
        // load texture from resource
        hr = DirectX::CreateWICTextureFromMemory(
            this->device.get(),
            reinterpret_cast<const uint8_t*>(tex->res.data()),
            tex->res.size(),
            texture_resource.put(),
            tex->texture_view.put()
        );
    }

    D3D11_TEXTURE2D_DESC desc = { };
    texture_resource.as<ID3D11Texture2D>()->GetDesc(&desc);
    tex->size = { static_cast<float>(desc.Width), static_cast<float>(desc.Height) };

    if (FAILED(hr)) {
        // handle error
    }
}

void renderer::ensure_mesh_buffers_created(const std::shared_ptr<mesh>& m) {
    if (m->vertex_buffer->size() != 0 && m->index_buffer->size() != 0) {
        return;
    }
    
    m->vertex_buffer->initialize(this->device, m->vertices.data(), m->vertices.size(), D3D11_BIND_VERTEX_BUFFER);
    m->index_buffer->initialize(this->device, m->indices.data(), m->indices.size(), D3D11_BIND_INDEX_BUFFER);
}
