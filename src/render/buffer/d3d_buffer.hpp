#pragma once
#include "common.hpp"
#include "cbuffer_types.hpp"

class d3d_buffer {
public:
    template <typename T>
    void initialize(const winrt::com_ptr<ID3D11Device>& device, const T* data, const size_t num_elements, const D3D11_BIND_FLAG bind_flag) {
        this->buffer_elements = num_elements;
        this->buffer_stride = std::make_unique<UINT>(sizeof(T));
        
        CD3D11_BUFFER_DESC desc(sizeof(T) * num_elements, bind_flag);
        D3D11_SUBRESOURCE_DATA buffer_data = { };
        buffer_data.pSysMem = data;

        HRESULT hr = device->CreateBuffer(&desc, &buffer_data, this->buffer.put());
        if (FAILED(hr)) {
            // handle error
        }
    }

    // specialized function for constant buffers
    template <typename T, typename = std::enable_if_t<std::is_base_of_v<cbuffer, T>>>
    void initialize(const winrt::com_ptr<ID3D11Device>& device) {
        CD3D11_BUFFER_DESC desc(static_cast<UINT>(sizeof(T) + (16 - sizeof(T) % 16)),
            D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

        HRESULT hr = device->CreateBuffer(&desc, nullptr, this->buffer.put());
        if (FAILED(hr)) {
            // handle error
        }
    }

    [[nodiscard]]

    winrt::com_ptr<ID3D11Buffer>& get() {
        return this->buffer;
    }

    [[nodiscard]] size_t size() const {
        return this->buffer_elements;
    }

    [[nodiscard]] UINT stride() const {
        return *this->buffer_stride;
    }

    [[nodiscard]] UINT* stride_ptr() const {
        return this->buffer_stride.get();
    }

private:
    winrt::com_ptr<ID3D11Buffer> buffer;
    size_t buffer_elements = 0;
    std::unique_ptr<UINT> buffer_stride;
};
