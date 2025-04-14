#pragma once
#include "common.hpp"

struct transform {
private:
    vector2 position = { 0, 0 };
    vector2 scale = { 1, 1 };
    float rotation = 0;
    
    bool dirty_matrix = true;
    matrix mat = DirectX::XMMatrixIdentity();

public:
    [[nodiscard]] vector2 get_position() const {
        return this->position;
    }

    [[nodiscard]] vector2 get_scale() const {
        return this->scale;
    }

    [[nodiscard]] float get_rotation() const {
        return this->rotation;
    }

    void set_position(const vector2& position) {
        this->position = position;
        this->dirty_matrix = true;
    }

    void set_scale(const vector2& scale) {
        this->scale = scale;
        this->dirty_matrix = true;
    }

    void set_rotation(const float rotation) {
        this->rotation = rotation;
        this->dirty_matrix = true;
    }

    // update matrix if needed to prevent unnecessary calculations
    const matrix& get_matrix() {
        if (this->dirty_matrix) {
            this->mat = DirectX::XMMatrixScaling(scale.x, scale.y, 1.0f) *
                DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(rotation)) *
                DirectX::XMMatrixTranslation(position.x + scale.x / 2, position.y + scale.y / 2, 0.0f);

            this->dirty_matrix = false;
        }

        return this->mat;
    }
};