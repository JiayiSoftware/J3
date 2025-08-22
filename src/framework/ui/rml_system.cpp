#include "rml_system.hpp"

#include <RmlUi/Debugger/Debugger.h>

#include "framework/resource/resource.hpp"

LOAD_RESOURCE(resources_fonts_Montserrat_ttf)
LOAD_RESOURCE(resources_fonts_MontserratItalic_ttf)

void rml_system::initialize(
    HWND handle, vector2 size, const winrt::com_ptr<ID3D11Device>& device,
    const winrt::com_ptr<ID3D11RenderTargetView>& rtv
) {
    this->window_handle = handle;
    this->window_size = Rml::Vector2i(static_cast<int>(size.x), static_cast<int>(size.y));
    this->device = device;
    this->render_target_view = rtv;

    this->render_interface = Rml::MakeUnique<dxtk_render_interface>(device);
    Rml::SetRenderInterface(this->render_interface.get());

    this->system_interface = Rml::MakeUnique<SystemInterface_Win32>();
    this->system_interface->SetWindow(this->window_handle);
    Rml::SetSystemInterface(this->system_interface.get());

    this->font_engine = Rml::MakeUnique<FontEngineInterfaceHarfBuzz>();
    Rml::SetFontEngineInterface(this->font_engine.get());

    this->file_interface = Rml::MakeUnique<embedded_file_interface>();
    Rml::SetFileInterface(this->file_interface.get());

    // add languages (BCP47 code for language, ISO15924 code for script... then text direction obviously)
    this->font_engine->RegisterLanguage("en", "Latn", TextFlowDirection::LeftToRight);

    this->ime = Rml::MakeUnique<TextInputMethodEditor_Win32>();
    Rml::SetTextInputHandler(this->ime.get());

    Rml::Initialise();
    this->context = Rml::CreateContext("main", this->window_size);

#ifndef NDEBUG // debug
    Rml::Debugger::Initialise(this->context);
#endif

    const resource montserrat = GET_RESOURCE(resources_fonts_Montserrat_ttf);
    const resource montserrat_italic = GET_RESOURCE(resources_fonts_MontserratItalic_ttf);

    Rml::LoadFontFace(
        Rml::Span(std::bit_cast<const unsigned char*>(montserrat.data()), montserrat.size()),
        Rml::String(),
        Rml::Style::FontStyle::Normal
    );
    Rml::LoadFontFace(
        Rml::Span(std::bit_cast<const unsigned char*>(montserrat_italic.data()), montserrat_italic.size()),
        Rml::String(),
        Rml::Style::FontStyle::Italic
    );

    const resource default_css = GET_RESOURCE(resources_ui_css_default_css);
    this->default_styles = Rml::Factory::InstanceStyleSheetString(Rml::String(default_css.str()));
}

void rml_system::update() {
    this->context->Update();
    this->render_interface->SetViewport(this->window_size.x, this->window_size.y);
    this->render_interface->BeginFrame(this->render_target_view.get());
    this->context->Render();
    this->render_interface->EndFrame();
}

void rml_system::destroy() {
    Rml::Shutdown();
    this->ime.reset();
}

void rml_system::resize(const vector2 new_size, const winrt::com_ptr<ID3D11RenderTargetView>& rtv) {
    this->window_size = { static_cast<int>(new_size.x), static_cast<int>(new_size.y) };
    this->context->SetDimensions(this->window_size);
    this->render_target_view = rtv;
}

bool rml_system::window_procedure(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param) {
    if (this->ime == nullptr) return false; // prevent access of null pointer after destruction

#ifndef NDEBUG // debug
    if (message == WM_KEYUP && w_param == VK_F12) {
        Rml::Debugger::SetVisible(!Rml::Debugger::IsVisible());
    }
#endif
    
    return !RmlWin32::WindowProcedure(this->context, *this->ime, window_handle, message, w_param, l_param);
}