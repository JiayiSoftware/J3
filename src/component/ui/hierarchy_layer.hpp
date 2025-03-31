#pragma once
#include "common.hpp"

struct hierarchy_layer {
private:
    entt::entity parent = entt::null;
    uint32_t layer = 0;
    bool dirty = false;

    // no vector of children because it gets expensive

public:
    [[nodiscard]] entt::entity get_parent() const {
        return this->parent;
    }

    void set_parent(const entt::entity parent) {
        this->parent = parent;
        this->dirty = true; // draw layer needs to be recalculated next frame
    }

    [[nodiscard]] uint32_t get_layer() const {
        return this->layer;
    }

    // this function should not be called outside the hierarchy system; changes will be lost if hierarchy is dirty
    void set_layer(const uint32_t layer) {
        this->layer = layer;
    }

    [[nodiscard]] bool is_dirty() const {
        return this->dirty;
    }
};
