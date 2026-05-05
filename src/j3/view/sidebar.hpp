#pragma once
#include "common.hpp"

#include "framework/ui/view/view.hpp"
#include "j3/controller/sidebar_controller.hpp"

class sidebar : public view<"sidebar", sidebar_controller> {
public:
    void after_load() override;
    void update() override;
};
