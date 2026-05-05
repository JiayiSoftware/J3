#pragma once
#include "common.hpp"

#include "framework/ui/controller/controller.hpp"
#include "j3/model/sidebar_model.hpp"

class sidebar_controller : public controller {
public:
    void bind_data(Rml::DataModelConstructor& dmc) override;
    
    void update();

private:
    sidebar_model model;

    void bring_to_front(Rml::DataModelHandle, Rml::Event& event, const Rml::VariantList&);
    void switch_tab(Rml::DataModelHandle, Rml::Event& event, const Rml::VariantList&);
};
