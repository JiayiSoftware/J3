#pragma once
#include "common.hpp"

#include "framework/ui/view/view.hpp"
#include "j3/controller/test_place_controller.hpp"

class test_place : public view<"test_place", test_place_controller> {
public:
    void after_load() override;
    void update() override;
};
