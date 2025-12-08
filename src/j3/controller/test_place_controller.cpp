#include "test_place_controller.hpp"

#include "framework/core/application.hpp"

void test_place_controller::bind_data(Rml::DataModelConstructor& dmc) {
    dmc.RegisterArray<std::vector<Rml::String>>();

    dmc.Bind("drop_down_name", &this->model.drop_down_name);
    dmc.Bind("drop_down_items", &this->model.drop_down_items);
    dmc.Bind("drop_down_selected_items", &this->model.drop_down_selected_items);
    dmc.Bind("drop_down_multiselect", &this->model.drop_down_multiselect);
    dmc.Bind("drop_down_open", &this->model.drop_down_open);
    dmc.Bind("drop_down_selected_items_formatted", &this->model.drop_down_selected_items_formatted);
    this->update_formatted_text();

    dmc.RegisterTransformFunc("get_item_id", [](const Rml::VariantList& args) -> Rml::Variant {
        if (args.empty()) return { };

        auto str = args[0].Get<Rml::String>();
        std::ranges::replace(str, ' ', '_');
        std::ranges::transform(str, str.begin(), tolower);

        return Rml::Variant{ str };
    });

    dmc.BindEventCallback("drop_down_toggle", &test_place_controller::toggle_drop_down_item, this);

    dmc.Bind("color_picker_open", &this->model.color_picker_open);

    // color will be represented as HSL
    if (auto vector_handle = dmc.RegisterStruct<vector4>()) {
        vector_handle.RegisterMember("hue", &vector4::x);
        vector_handle.RegisterMember("saturation", &vector4::y);
        vector_handle.RegisterMember("lightness", &vector4::z);
        vector_handle.RegisterMember("alpha", &vector4::w);
    }
    
    dmc.Bind("color_picker_color", &this->model.color_picker_color);
    dmc.Bind("color_picker_hex", &this->model.color_picker_hex);

    dmc.BindEventCallback("color_picker_update", &test_place_controller::update_color_picker_color, this);
    
    this->handle = dmc.GetModelHandle();
}

void test_place_controller::toggle_drop_down_item(
    Rml::DataModelHandle handle, Rml::Event& e, const Rml::VariantList& args
) {
    if (args.empty()) return;
    
    Rml::Element* element = e.GetTargetElement();
    if (element == nullptr) return;

    auto str = args[0].Get<Rml::String>();

    if (element->HasAttribute("checked")) {
        if (!this->model.drop_down_multiselect) this->model.drop_down_selected_items.clear();

        this->model.drop_down_selected_items.push_back(str);
    } else {
        this->model.drop_down_selected_items.erase(std::ranges::find(this->model.drop_down_selected_items, str));
    }

    handle.DirtyVariable("drop_down_selected_items");
    this->update_formatted_text();
}

void test_place_controller::update_formatted_text() {
    if (this->model.drop_down_selected_items.empty()) return;
    
    if (this->model.drop_down_selected_items.size() == 1) {
        this->model.drop_down_selected_items_formatted = this->model.drop_down_selected_items[0];
    } else {
        this->model.drop_down_selected_items_formatted = fmt::format(
            "{}, +{}...",
            this->model.drop_down_selected_items[0],
            this->model.drop_down_selected_items.size() - 1
        );
    }
    
    this->handle.DirtyVariable("drop_down_selected_items_formatted");
}

void test_place_controller::update_color_picker_color(
    Rml::DataModelHandle handle, Rml::Event& e, const Rml::VariantList& args
) {
    Rml::Element* element = e.GetTargetElement();
    if (element == nullptr) return;

    if (element->GetTagName() != "handle") {
        Rml::String type = element->GetAttribute("type", Rml::String());
        if (type.empty()) return;

        if (type == "range") {
            // just update hex code, the lines after are only for the color zone
            this->model.color_picker_hex = this->hsla_to_hex(this->model.color_picker_color);
            handle.DirtyVariable("color_picker_hex");
            return;
        }

        if (type == "text") {
            // just update HSL color using received hex
            Rml::String hex = e.GetParameter("value", Rml::String());
            if (hex.empty()) return;
            if (!std::regex_search(hex, this->valid_hex)) return;

            this->model.color_picker_color = this->hex_to_hsla(hex);
            handle.DirtyVariable("color_picker_color");
            
            Rml::Element* zone = element->GetParentNode()->GetElementById("color-picker-zone");
            Rml::Element* marker = zone->GetElementById("color-picker-marker");
            this->update_color_picker_marker(marker, zone);
            return;
        }
    }
    
    float x = e.GetParameter("mouse_x", 0.f);
    float y = e.GetParameter("mouse_y", 0.f);

    Rml::Element* color_zone = element->GetParentNode();
    if (color_zone == nullptr) return;
    
    x -= color_zone->GetAbsoluteLeft();
    y -= color_zone->GetAbsoluteTop();
    x = std::clamp(x, 0.f, color_zone->GetClientWidth());
    y = std::clamp(y, 0.f, color_zone->GetClientHeight());

    float s = x / color_zone->GetClientWidth();
    float v = 1 - y / color_zone->GetClientHeight();

    // need to convert HSV to HSL for rml
    float l = v * (1 - s / 2);
    float sl = l == 0 || l == 1 ? 0 : (v - l) / std::min(l, 1 - l);

    this->model.color_picker_color.y = sl * 100;
    this->model.color_picker_color.z = l * 100;

    this->model.color_picker_hex = this->hsla_to_hex(this->model.color_picker_color);

    handle.DirtyVariable("color_picker_color");
    handle.DirtyVariable("color_picker_hex");
}

void test_place_controller::update_color_picker_marker(Rml::Element* marker, Rml::Element* zone) {
    // convert HSL back to HSV
    float s = this->model.color_picker_color.y / 100;
    float l = this->model.color_picker_color.z / 100;
    
    float v = l + s * std::min(l, 1 - l);
    float sv = v == 0 ? 0 : 2 * (1 - l / v);
    
    float x = sv * zone->GetClientWidth();
    float y = (1 - v) * zone->GetClientHeight();
    
    // account for marker center (hardcoded sorry)
    // x -= marker->GetClientWidth() / 2;
    // y -= marker->GetClientHeight() / 2;
    
    x -= 9;
    y -= 9;
    
    marker->SetProperty("left", fmt::format("{}px", static_cast<int>(x)));
    marker->SetProperty("top", fmt::format("{}px", static_cast<int>(y)));
}

std::string test_place_controller::hsla_to_hex(const vector4 hsla) {
    // hsla to rgba
    float h = hsla.x;
    float s = hsla.y / 100.f;
    float l = hsla.z / 100.f;
    float a = hsla.w;
    
    float c = (1.f - std::abs(2.f * l - 1.f)) * s;
    float x = c * (1.f - std::abs(std::remainder(h / 60.f, 2.f) - 1.f));
    float m = l - c / 2.f;

    float r, g, b;

    if (h < 60) {
        r = c;
        g = x;
        b = 0;
    } else if (h < 120) {
        r = x;
        g = c;
        b = 0;
    } else if (h < 180) {
        r = 0;
        g = c;
        b = x;
    } else if (h < 240) {
        r = 0;
        g = x;
        b = c;
    } else if (h < 300) {
        r = x;
        g = 0;
        b = c;
    } else {
        r = c;
        g = 0;
        b = x;
    }

    vector4 rgba = { (r + m) * 255.f, (g + m) * 255.f, (b + m) * 255.f, a * 255.f };

    // rgb to hex
    return fmt::format("#{:02X}{:02X}{:02X}{:02X}", static_cast<uint8_t>(rgba.x), static_cast<uint8_t>(rgba.y), static_cast<uint8_t>(rgba.z), static_cast<uint8_t>(rgba.w));
}

vector4 test_place_controller::hex_to_hsla(const std::string& hex) {
    // hex to rgba
    std::string hex_no_tag = hex.substr(1);
    uint32_t rgba = std::stoul(hex_no_tag, nullptr, 16);

    float r, g, b, a;
    if (hex_no_tag.length() == 8) {
        r = (rgba >> 24 & 0xFF) / 255.f;
        g = (rgba >> 16 & 0xFF) / 255.f;
        b = (rgba >> 8 & 0xFF) / 255.f;
        a = (rgba & 0xff) / 255.f;
    } else { // has to be of size 7
        r = (rgba >> 16 & 0xFF) / 255.f;
        g = (rgba >> 8 & 0xFF) / 255.f;
        b = (rgba & 0xFF) / 255.f;
        a = 1.f;
    }

    // rgba to hsla
    float l = std::max({ r, g, b });
    float s = l - std::min({ r, g, b });
    float h = s
        ? l == r
            ? (g - b) / s
            : l == g
            ? 2.f + (b - r) / s
            : 4.f + (r - g) / s
        : 0.f;

    return {
        60.f * h < 0 ? 60.f * h + 360.f : 60.f * h,
        100.f * (s ? (l <= 0.5f ? s / (2.f * l - s) : s / (2.f - (2.f * l - s))) : 0.f),
        100.f * (2.f * l - s) / 2.f,
        a
    };
}