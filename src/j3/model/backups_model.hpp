#pragma once
#include "common.hpp"

struct backups_model {
    Rml::String current_version{ "Unknown" };
    int current_w{ 0 };
    int current_rp{ 0 };
    int current_bp{ 0 };
};
