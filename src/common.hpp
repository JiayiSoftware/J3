#pragma once

// windows related includes
#include <Windows.h>
#include <dwmapi.h>
#include <ShlObj.h>
#include <d3d11.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <winrt/base.h>
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
#include <regex>

// libraries
#include <DirectXMath.h>
#include <RmlUi/Core.h>
#include <entt/entt.hpp>
#include <libhat.hpp>
#include <spdlog/spdlog.h>
#include <glaze/json.hpp>

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
        spdlog::level("{} with HRESULT: 0x{:08X}", reason, hr); \
    }
// LOG_HRESULT
