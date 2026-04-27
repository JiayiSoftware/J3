#pragma once
#include "common.hpp"

#include "framework/resource/resource.hpp"
#include "interface/render/dxtk_render_interface.hpp"
#include "interface/system/l10n_system_interface.hpp"

#include <RmlUi/Core/Input.h>
#include <RmlUi_Platform_Win32.h>
#include <FontEngineInterfaceHarfBuzz.h>

struct rml_system {
    void initialize(
        HWND handle, vector2 size, const winrt::com_ptr<ID3D11Device>& device,
        const winrt::com_ptr<ID3D11RenderTargetView>& rtv
    );

    void update();
    void request_update(double timeout = 0.0);
    void destroy();

    void resize(vector2 new_size, const winrt::com_ptr<ID3D11RenderTargetView>& rtv);
    void set_dip_ratio(float ratio);
    bool window_procedure(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param);

    template <typename view_t>
    void register_view();

    template <typename view_t>
    void show_view();

    template <typename view_t>
    void hide_view();

private:
    class refresh_listener : public Rml::EventListener {
    public:
        void ProcessEvent(Rml::Event& event) override {
            Rml::Input::KeyIdentifier key = event.GetParameter("key_identifier", Rml::Input::KeyIdentifier::KI_UNKNOWN);
            if (key == Rml::Input::KI_F5) {
                event.GetTargetElement()->GetOwnerDocument()->ReloadStyleSheet();
            }
        }
    };

    template <typename view_t>
    struct storage {
        static inline view_t view;
        static inline Rml::ElementDocument* document = nullptr;
        static inline refresh_listener refresh_listener;
    };

    HWND window_handle = nullptr;
    Rml::Vector2i window_size = Rml::Vector2i();

    winrt::com_ptr<ID3D11Device> device;
    winrt::com_ptr<ID3D11RenderTargetView> render_target_view;

    Rml::UniquePtr<dxtk_render_interface> render_interface;
    Rml::UniquePtr<l10n_system_interface> system_interface;
    Rml::UniquePtr<FontEngineInterfaceHarfBuzz> font_engine;

    Rml::Context* context = nullptr;
    Rml::UniquePtr<TextInputMethodEditor_Win32> ime;
};

template <typename view_t>
void rml_system::register_view() {
    Rml::DataModelConstructor dmc = this->context->CreateDataModel(Rml::String(view_t::name.c_str()) + "_data");
    storage<view_t>::view.initialize(dmc);

    Rml::ElementDocument* document = this->context->LoadDocument(view_t::path.str());
    if (!document) return;

    document->AddEventListener(Rml::EventId::Keydown, &storage<view_t>::refresh_listener);

    storage<view_t>::document = document;
    storage<view_t>::view.after_load();
}

template <typename view_t>
void rml_system::show_view() {
    if (storage<view_t>::document == nullptr) return;

    storage<view_t>::document->Show();
}

template <typename view_t>
void rml_system::hide_view() {
    if (storage<view_t>::document == nullptr) return;

    storage<view_t>::document->Hide();
}
