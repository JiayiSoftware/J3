#include "dxtk_render_interface.hpp"

#include "framework/core/application.hpp"

#include <WICTextureLoader.h>

dxtk_render_interface::dxtk_render_interface(const winrt::com_ptr<ID3D11Device>& device) {
    this->device = device;
    this->Init(this->device.get());
}
Rml::TextureHandle dxtk_render_interface::LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) {
    Rml::FileInterface* file_interface = Rml::GetFileInterface();
    Rml::FileHandle file_handle = file_interface->Open(source);
    if (!file_handle) {
        return false;
    }

    file_interface->Seek(file_handle, 0, SEEK_END);
    size_t buffer_size = file_interface->Tell(file_handle);
    file_interface->Seek(file_handle, 0, SEEK_SET);

    Rml::UniquePtr<std::byte[]> buffer(new std::byte[buffer_size]);
    file_interface->Read(buffer.get(), buffer_size, file_handle);
    file_interface->Close(file_handle);

    winrt::com_ptr<ID3D11Resource> texture;
    ID3D11ShaderResourceView* texture_srv;

    HRESULT hr =
        DirectX::CreateWICTextureFromMemory(this->device.get(), buffer.get(), buffer_size, texture.put(), &texture_srv);

    LOG_HRESULT(error, "Loading image for RML failed", hr);

    D3D11_TEXTURE2D_DESC desc = {};
    texture.as<ID3D11Texture2D>()->GetDesc(&desc);
    texture_dimensions.x = desc.Width;
    texture_dimensions.y = desc.Height;

    return std::bit_cast<Rml::TextureHandle>(texture_srv);
}