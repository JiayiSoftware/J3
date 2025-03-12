#include "renderer.hpp"

void renderer::initialize(const HWND handle, const vector2 size, const bool hardware_accelerated) {
    this->window_handle = handle;
    this->window_size = size;
    
    create_device_and_swap_chain(hardware_accelerated);
    create_render_target();
}

void renderer::render_frame() {
    float background[] = { 0.0f, 0.0f, 1.0f, 1.0f };
    this->device_context->ClearRenderTargetView(this->render_target_view.get(), background);

    // render stuff here

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

    ID3D11RenderTargetView* render_target_view = this->render_target_view.get(); // instead of winrt::com_ptr::put
    this->device_context->OMSetRenderTargets(1, &render_target_view, nullptr);
}
