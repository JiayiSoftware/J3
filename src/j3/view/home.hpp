#pragma once
#include "common.hpp"

#include "framework/ui/view/view.hpp"
#include "j3/controller/home_controller.hpp"

class home : public view<"home", home_controller> {
public:
    void after_load() override;
};