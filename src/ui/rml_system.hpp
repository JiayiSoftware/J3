#pragma once
#include "common.hpp"

#include "page.hpp"
#include "font/FontEngineInterfaceHarfBuzz.h"

// no MSAA for RmlUi, we do this ourselves already
#define NUM_MSAA_SAMPLES 1
#include <RmlUi_Renderer_DX11.h>
#include <RmlUi_Platform_Win32.h>

struct rml_system {
    void initialize(HWND handle, vector2 size, const winrt::com_ptr<ID3D11Device>& device, const winrt::com_ptr<ID3D11RenderTargetView>& rtv);
    void update();
    void destroy();

    template <typename page_t, typename... args>
    void register_page(args&&... a);

    template <typename page_t>
    void show_page();

    template <typename page_t>
    void hide_page();

private:
    using page_hash = std::uint32_t;
    
    HWND window_handle = nullptr;
    Rml::Vector2i window_size = Rml::Vector2i();
    
    winrt::com_ptr<ID3D11Device> device;
    winrt::com_ptr<ID3D11RenderTargetView> render_target_view;
    
    RenderInterface_DX11 render_interface;
    SystemInterface_Win32 system_interface;

    Rml::Context* context = nullptr;
    Rml::SharedPtr<Rml::StyleSheetContainer> default_styles;
    Rml::UniquePtr<FontEngineInterfaceHarfBuzz> font_engine;
    std::unordered_map<page_hash, Rml::ElementDocument*> document_map;
    
    Rml::ElementDocument* init_page(page& p) const;
    std::string_view get_default_styles_str();
};

template <typename page_t, typename ... args>
void rml_system::register_page(args&&... a) {
    page p{page_t(std::forward<args>(a)...)};
    auto* doc = init_page(p);
    if (doc == nullptr) return;

    static constexpr page_hash hash = entt::type_hash<page_t>::value();
    this->document_map[hash] = doc;
}

template <typename page_t>
void rml_system::show_page() {
    static constexpr page_hash hash = entt::type_hash<page_t>::value();
    if (!this->document_map.contains(hash)) return;

    this->document_map[hash]->Show();
}

template <typename page_t>
void rml_system::hide_page() {
    static constexpr page_hash hash = entt::type_hash<page_t>::value();
    if (!this->document_map.contains(hash)) return;

    this->document_map[hash]->Hide();
}
