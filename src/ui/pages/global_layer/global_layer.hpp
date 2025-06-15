#pragma once
#include "common.hpp"

struct global_layer {
    std::string_view id();
    std::string_view layout();
    std::string_view styles();
    void bind_data(Rml::DataModelConstructor& dmc) {};
};
