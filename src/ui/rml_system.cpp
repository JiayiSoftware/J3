#include "rml_system.hpp"

#include <RmlUi/Debugger/Debugger.h>

#include "core/application.hpp"
#include "resource/resource.hpp"

LOAD_RESOURCE(resources_fonts_Montserrat_ttf)
LOAD_RESOURCE(resources_fonts_MontserratItalic_ttf)

void rml_system::initialize(HWND handle, vector2 size, const winrt::com_ptr<ID3D11Device>& device,
    const winrt::com_ptr<ID3D11RenderTargetView>& rtv) {
    this->window_handle = handle;
    this->window_size = Rml::Vector2i(
        static_cast<int>(size.x),
        static_cast<int>(size.y)
    ); 
    this->device = device;
    this->render_target_view = rtv;
    
    this->render_interface.Init(this->device.get());
    this->system_interface.SetWindow(this->window_handle);

    Rml::SetRenderInterface(&this->render_interface);
    Rml::SetSystemInterface(&this->system_interface);

    this->font_engine = Rml::MakeUnique<FontEngineInterfaceHarfBuzz>();
    Rml::SetFontEngineInterface(this->font_engine.get());

    // add languages (BCP47 code for language, ISO15924 code for script... then text direction obviously)
    this->font_engine->RegisterLanguage("en", "Latn", TextFlowDirection::LeftToRight);

    Rml::Initialise();
    this->context = Rml::CreateContext("main", this->window_size);

    //Rml::Debugger::Initialise(this->context);

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

    this->default_styles = Rml::Factory::InstanceStyleSheetString(Rml::String(this->get_default_styles_str()));

    //Rml::Debugger::SetVisible(true);
}

void rml_system::update() {
    // TODO: input

    this->context->Update();
    this->render_interface.SetViewport(this->window_size.x, this->window_size.y);
    this->render_interface.BeginFrame(this->render_target_view.get());
    this->context->Render();
    this->render_interface.EndFrame();
}

void rml_system::destroy() {
    Rml::Shutdown();
}

Rml::ElementDocument* rml_system::init_page(page& p) const {
    auto& app = application::get();

    Rml::ElementDocument* doc = this->context->LoadDocumentFromMemory(Rml::String(p->layout()), Rml::String(p->id()));
    if (doc == nullptr) {
        app.log.error("'{}' page failed to initialize", p->id());
        return nullptr;
    }
    
    Rml::SharedPtr<Rml::StyleSheetContainer> custom_styles = Rml::Factory::InstanceStyleSheetString(Rml::String(p->styles()));
    Rml::SharedPtr<Rml::StyleSheetContainer> combined_styles = this->default_styles->CombineStyleSheetContainer(*custom_styles);
    doc->SetStyleSheetContainer(combined_styles);
    
    app.log.debug("'{}' page initialized", p->id());
    return doc;
}

std::string_view rml_system::get_default_styles_str() {
    // source: https://github.com/mikke89/RmlUi/blob/master/Samples/assets/rml.rcss
    return R"(
        /*
        * Default styles for all the basic elements.
        */

        div
        {
	        display: block;
        }

        p
        {
	        display: block;
        }

        h1
        {
	        display: block;
        }

        em
        {
	        font-style: italic;
        }

        strong
        {
	        font-weight: bold;
        }

        select
        {
	        text-align: left;
        }

        tabset tabs
        {
	        display: block;
        }

        table {
	        box-sizing: border-box;
	        display: table;
        }

        tr {
	        box-sizing: border-box;
	        display: table-row;
        }

        td {
	        box-sizing: border-box;
	        display: table-cell;
        }

        col {
	        box-sizing: border-box;
	        display: table-column;
        }

        colgroup {
	        display: table-column-group;
        }

        thead, tbody, tfoot {
	        display: table-row-group;
        }
    )";
}
