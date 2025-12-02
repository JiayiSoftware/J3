#pragma once
#include "common.hpp"

struct test_place_model {
    Rml::String drop_down_name{ "Drop down box" };
    std::vector<Rml::String> drop_down_items{ "First option", "Second option", "Third option" };
    std::vector<Rml::String> drop_down_selected_items{ }; // can have only one item if not multiselect
    Rml::String drop_down_selected_items_formatted{ };
    bool drop_down_multiselect{ true };
    bool drop_down_open{ false };

    bool color_picker_open{ false };
};
