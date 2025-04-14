#pragma once

// windows related includes
#include <Windows.h>
#include <ShlObj.h>
#include <d3d11.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <winrt/windows.foundation.h>

// standard includes
#include <random>
#include <future>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string>
#include <span>
#include <thread>
#include <vector>
#include <utility>
#include <memory>
#include <print>
#include <mutex>
#include <array>
#include <filesystem>

// libraries
#include <entt/entt.hpp>
#include <spdlog/spdlog.h>
#include <DirectXMath.h>

// aliases for dxmath because the names are silly
using vector = DirectX::XMVECTOR;
using vector2 = DirectX::XMFLOAT2;
using vector3 = DirectX::XMFLOAT3;
using vector4 = DirectX::XMFLOAT4;
using matrix = DirectX::XMMATRIX;
using matrix4x3 = DirectX::XMFLOAT4X3;
using matrix4x4 = DirectX::XMFLOAT4X4;
using quaternion = DirectX::XMFLOAT4;

// helper macro for logging an HRESULT
    #define LOG_HRESULT(level, reason, hr) \
    if (FAILED(hr)) { \
        application::get().log.level("{} with HRESULT: 0x{:08X}", reason, hr); \
    }
// LOG_HRESULT
