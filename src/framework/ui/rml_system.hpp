#pragma once
#include "common.hpp"

#include "font/FontEngineInterfaceHarfBuzz.h"
#include "framework/resource/resource.hpp"

// no MSAA for RmlUi, we do this ourselves already
#define NUM_MSAA_SAMPLES 1
#include <RmlUi_Platform_Win32.h>
#include <RmlUi_Renderer_DX11.h>

LOAD_RESOURCE(resources_ui_css_default_css)

struct rml_system {
    void initialize(
        HWND handle, vector2 size, const winrt::com_ptr<ID3D11Device>& device,
        const winrt::com_ptr<ID3D11RenderTargetView>& rtv
    );
    void update();
    void destroy();

    void resize(vector2 new_size, const winrt::com_ptr<ID3D11RenderTargetView>& rtv);
    bool window_procedure(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param);

    template <typename page_t>
    void register_page();

    template <typename page_t>
    void show_page();

    template <typename page_t>
    void hide_page();

private:
    template <typename page_t>
    struct storage {
        static inline page_t page;
        static inline Rml::ElementDocument* document = nullptr;
    };

    HWND window_handle = nullptr;
    Rml::Vector2i window_size = Rml::Vector2i();

    winrt::com_ptr<ID3D11Device> device;
    winrt::com_ptr<ID3D11RenderTargetView> render_target_view;

    RenderInterface_DX11 render_interface;
    SystemInterface_Win32 system_interface;

    Rml::Context* context = nullptr;
    Rml::SharedPtr<Rml::StyleSheetContainer> default_styles;
    Rml::UniquePtr<FontEngineInterfaceHarfBuzz> font_engine;
    Rml::UniquePtr<TextInputMethodEditor_Win32> ime;
};

template <typename page_t>
void rml_system::register_page() {
    new (&storage<page_t>::page) page_t();

    Rml::DataModelConstructor dmc = this->context->CreateDataModel(Rml::String(page_t::name.c_str()) + "_data");
    storage<page_t>::page.initialize(dmc);

    Rml::ElementDocument* document =
        this->context->LoadDocumentFromMemory(Rml::String(page_t::rml().str()), page_t::name.c_str());
    if (!document) return;

    const Rml::SharedPtr<Rml::StyleSheetContainer> custom_styles =
        Rml::Factory::InstanceStyleSheetString(Rml::String(page_t::css().str()));
    const Rml::SharedPtr<Rml::StyleSheetContainer> combined_styles =
        this->default_styles->CombineStyleSheetContainer(*custom_styles);
    document->SetStyleSheetContainer(combined_styles);

    storage<page_t>::document = document;
    storage<page_t>::page.after_load();
}

template <typename page_t>
void rml_system::show_page() {
    if (storage<page_t>::document == nullptr) return;

    storage<page_t>::document->Show();
}

template <typename page_t>
void rml_system::hide_page() {
    if (storage<page_t>::document == nullptr) return;

    storage<page_t>::document->Hide();
}
