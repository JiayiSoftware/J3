#pragma once
#include "common.hpp"

#include "framework/ui/view/view.hpp"
#include "j3/controller/settings_controller.hpp"

class settings : public view<"settings", settings_controller> {
public:
    void after_load() override;
};
