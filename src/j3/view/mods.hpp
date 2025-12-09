#pragma once
#include "common.hpp"

#include "framework/ui/view/view.hpp"
#include "j3/controller/mods_controller.hpp"

class mods : public view<"mods", mods_controller> {
public:
    void after_load() override;
};
