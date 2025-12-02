#include "test_place_controller.hpp"

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