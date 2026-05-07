#pragma once
#include "common.hpp"

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

    void update() const;
    void request_update(double timeout = 0.0) const;
    void destroy();

    void resize(vector2 new_size, const winrt::com_ptr<ID3D11RenderTargetView>& rtv);
    void set_dip_ratio(float ratio) const;
    bool window_procedure(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param) const;

    template <typename view_t>
    void register_view();

    template <typename view_t>
    void show_view();

    template <typename view_t>
    void hide_view();
    
    template <typename view_t>
    void update_view();
    
    template <typename view_t>
    void pull_to_front();

private:
    // TODO: reload entire page on F5 instead (or even add hot reloading)
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

    HWND window_handle{ nullptr };
    Rml::Vector2i window_size{ };

    winrt::com_ptr<ID3D11Device> device{ nullptr };
    winrt::com_ptr<ID3D11RenderTargetView> render_target_view{ nullptr };

    Rml::UniquePtr<dxtk_render_interface> render_interface{ nullptr };
    Rml::UniquePtr<l10n_system_interface> system_interface{ nullptr };
    Rml::UniquePtr<FontEngineInterfaceHarfBuzz> font_engine{ nullptr };

    Rml::Context* context{ nullptr };
    Rml::UniquePtr<TextInputMethodEditor_Win32> ime{ nullptr };
    
    std::vector<std::unique_ptr<Rml::ElementDocument>> always_on_top_documents;
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

template <typename view_t>
void rml_system::update_view() {
    storage<view_t>::view.update();
}

template <typename view_t>
void rml_system::pull_to_front() {
    if (storage<view_t>::document == nullptr) return;
    
    storage<view_t>::document->PullToFront();
}
