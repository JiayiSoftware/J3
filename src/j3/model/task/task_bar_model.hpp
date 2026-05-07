#pragma once
#include "common.hpp"

class task_bar_model {
public:
    Rml::String task_status{ "Initializing..." };
    float total_progress{ 0.1f };
    bool shown{ false };
};
