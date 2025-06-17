#include "rml_system.hpp"

#include <RmlUi/Debugger/Debugger.h>

#include "core/application.hpp"
#include "resource/resource.hpp"

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

    this->render_interface.Init(this->device.get());
    this->system_interface.SetWindow(this->window_handle);

    Rml::SetRenderInterface(&this->render_interface);
    Rml::SetSystemInterface(&this->system_interface);

    this->font_engine = Rml::MakeUnique<FontEngineInterfaceHarfBuzz>();
    Rml::SetFontEngineInterface(this->font_engine.get());

    // add languages (BCP47 code for language, ISO15924 code for script... then text direction obviously)
    this->font_engine->RegisterLanguage("en", "Latn", TextFlowDirection::LeftToRight);

    this->ime = Rml::MakeUnique<TextInputMethodEditor_Win32>();
    Rml::SetTextInputHandler(this->ime.get());

    Rml::Initialise();
    this->context = Rml::CreateContext("main", this->window_size);

    // Rml::Debugger::Initialise(this->context);

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

    // Rml::Debugger::SetVisible(true);
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
    this->ime.reset();
}

void rml_system::resize(const vector2 new_size, const winrt::com_ptr<ID3D11RenderTargetView>& rtv) {
    this->window_size = { static_cast<int>(new_size.x), static_cast<int>(new_size.y) };
    this->context->SetDimensions(this->window_size);
    this->render_target_view = rtv;
}

bool rml_system::window_procedure(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param) {
    if (this->ime == nullptr) return false; // prevent access of null pointer after destruction

    return !RmlWin32::WindowProcedure(this->context, *this->ime, window_handle, message, w_param, l_param);
}

Rml::ElementDocument* rml_system::init_page(page& p) const {
    auto& app = application::get();

    Rml::DataModelConstructor dmc = this->context->CreateDataModel(Rml::String(p->id()) + "_data");
    if (!dmc) {
        app.log.error("Data model creation failed for page {}", p->id());
        return nullptr;
    }

    p->bind_data(dmc);

    Rml::ElementDocument* doc = this->context->LoadDocumentFromMemory(Rml::String(p->layout()), Rml::String(p->id()));
    if (doc == nullptr) {
        app.log.error("Document failed to load for page {}", p->id());
        return nullptr;
    }

    Rml::SharedPtr<Rml::StyleSheetContainer> custom_styles =
        Rml::Factory::InstanceStyleSheetString(Rml::String(p->styles()));
    Rml::SharedPtr<Rml::StyleSheetContainer> combined_styles =
        this->default_styles->CombineStyleSheetContainer(*custom_styles);
    doc->SetStyleSheetContainer(combined_styles);

    app.log.debug("'{}' page initialized", p->id());
    return doc;
}

std::string_view rml_system::get_default_styles_str() {
    return R"(
        /*
		* Default styles for all the basic elements.
		*/

		div {
			display: block;
		}

		body {
            background-color: #0F0F0F00;
            color: #D9D9D9;
			font-family: Montserrat;
			font-size: 16px;
			font-weight: 600;
		    width: 100vw;
		    height: 100vh;
		    display: flex;
			padding: 15px 15px 15px 94px;
		    flex-direction: column;
		}

		title {
			font-family: Montserrat;

		}

		.fill {
			flex: 1;
		}

		input.range {
			display: block;
			width: 100%;
			background-color: #222;
		}


		input.range slidertrack {
			margin-top: 3dp;
			background-color: #919191;
		}

		input.range sliderbar {
			width: 34dp;
			background-color: #51357e;
			border-radius: 20dp;
		}

		scrollbarvertical {
			width: 20dp;
			background-color: #FFF;
		}

		scrollbarvertical sliderbar {
			width: 20dp;
			background-color: #000000;
		}

		scrollbarhorizontal {
			height: 20dp;
			background-color: #FFF;
		}

		scrollbarhorizontal sliderbar {
			height: 20dp;
			background-color: #000000;
		}

		scrollbarhorizontal slidertrack {
			background-color: #FDD;
		}

		em {
			font-style: italic;
		}

		strong {
			font-weight: bold;
		}

		select {
			text-align: left;
		}

		tabset tabs {
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

		thead,
		tbody,
		tfoot {
			display: table-row-group;
		}

		body,
		div,
		h1,
		h2,
		h3,
		h4,
		h5,
		h6,
		p,
		hr,
		pre,
		tabset tabs {
			display: block;
		}

		h1 {
			font-size: 24px;
			font-weight: 800;
            margin-bottom: 15px;
		}

		h2 {
			font-size: 20px;
			font-weight: 700;
            margin-bottom: 15px;
		}

		h3 {
            color: #6C6C6C;
			font-size: 18px;
			font-weight: 700;
            margin-bottom: 15px;
		}

		p {
			margin-bottom: 15px;
		}

        i {
			font-weight: 500;
            color: #6C6C6C;
			margin-bottom: 15px;
        }

		em {
            color: #6C6C6C;
			font-weight: 400;
			font-style: italic;
            font-size: 12px;
			margin-bottom: 15px;
		}

		span {
			white-space: nowrap;
		}

		pre {
			white-space: pre;
		}

		hr {
			border-width: 1dp;
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

		thead,
		tbody,
		tfoot {
			display: table-row-group;
		}

        @keyframes button-pressed {
            from {
                filter: drop-shadow(#0000003F 0 4px 2px) brightness(250%);
            }

            to {
                filter: drop-shadow(#0000003F 0 4px 2px) brightness(150%);
            }
        }

		button {
			background-color: #CE3A3A;
            border: 2px #6A1A1A;
            padding: 7px 9px 7px 9px;
            border-radius: 4px;
            tab-index: auto;
            cursor: pointer;
            box-sizing: border-box;
			font-weight: 700;
            filter: drop-shadow(#0000003F 0 4px 2px) brightness(100%);
            transition: all 0.5s quintic-out;
		}

		button:hover {
			filter: drop-shadow(#0000003F 0 4px 2px) brightness(120%);
		}

        button:active {
            transition: none;
            filter: drop-shadow(#0000003F 0 4px 2px) brightness(150%);
            animation: 0.5s quintic-out button-pressed;
        }

        button.less {
			font-weight: 500;
            background-color: transparent;
            filter: none;
        }

		button.less:hover {
			filter: brightness(120%);
		}

        button.less:active {
            transition: none;
            filter: drop-shadow(#0000003F 0 4px 2px) brightness(150%);
            animation: 0.5s quintic-out button-pressed;
        }


        button.disabled {
            font-weight: 500;
            background-color: transparent;
            border-color: #6C6C6C;
            color: #6C6C6C;
            cursor: unavailable;
            filter: none;
        }

        button.disabled:hover {
            filter: none;
        }

        button.disabled:active {
            animation: none;
        }

        button.progress {
            cursor: wait;
        }

		input.text {
            background-color: #FFFFFF;
            color: #000000;
            border: 2px #999999;
            padding: 5px;
            tab-index: auto;
            cursor: text;
            box-sizing: border-box;
            width: 200px;
			font-size: 14px;
        }
    )";
}
