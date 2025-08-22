#pragma once
#include "common.hpp"

#define NUM_MSAA_SAMPLES 1
#include <RmlUi_Renderer_DX11.h>

class dxtk_render_interface : public RenderInterface_DX11 {
public:
    dxtk_render_interface(const winrt::com_ptr<ID3D11Device>& device);

    Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) override;

private:
    winrt::com_ptr<ID3D11Device> device;
};
