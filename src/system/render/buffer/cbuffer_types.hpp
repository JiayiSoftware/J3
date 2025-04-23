#pragma once
#include "common.hpp"

struct cbuffer { }; // base struct for constant buffers

struct cb_vertex : cbuffer {
    matrix mat;
};

struct cb_pixel : cbuffer {
    float has_color;
};
