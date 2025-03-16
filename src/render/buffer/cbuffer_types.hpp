#pragma once
#include "common.hpp"

struct cbuffer { }; // base struct for constant buffers

struct cb_vertex : cbuffer {
    float x_offset;
    float y_offset;
};
