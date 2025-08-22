#pragma once
#include "common.hpp"

#include "framework/resource/resource.hpp"
#include "framework/ui/view/view.hpp"
#include "j3/controller/sidebar_controller.hpp"

LOAD_RESOURCE(resources_ui_css_sidebar_css)
LOAD_RESOURCE(resources_ui_rml_sidebar_rml)

class sidebar : public view<sidebar_controller> {
public:
    static constexpr hat::fixed_string name = "sidebar";
    static resource css() { return GET_RESOURCE(resources_ui_css_sidebar_css); }
    static resource rml() { return GET_RESOURCE(resources_ui_rml_sidebar_rml); }

    sidebar();

    void after_load() override;
};
