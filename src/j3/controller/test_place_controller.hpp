#pragma once
#include "common.hpp"

#include "framework/ui/controller/controller.hpp"
#include "j3/model/test_place_model.hpp"

class test_place_controller : public controller {
public:
    void bind_data(Rml::DataModelConstructor& dmc) override;
    
private:
    test_place_model model;
    Rml::DataModelHandle handle;
    std::regex valid_hex{ "^#([a-fA-F0-9]{8}|[a-fA-F0-9]{6})$" };

    void toggle_drop_down_item(Rml::DataModelHandle handle, Rml::Event& e, const Rml::VariantList& args);
    void update_formatted_text();

    void update_color_picker_color(Rml::DataModelHandle handle, Rml::Event& e, const Rml::VariantList& args);
    void update_color_picker_marker(Rml::Element* marker, Rml::Element* zone);
    std::string hsla_to_hex(vector4 hsla);
    vector4 hex_to_hsla(const std::string& hex);
};
