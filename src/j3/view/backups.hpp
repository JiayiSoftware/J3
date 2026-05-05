#pragma once
#include "common.hpp"

#include "framework/ui/view/view.hpp"
#include "j3/controller/backups_controller.hpp"

class backups : public view<"backups", backups_controller> {
public:
    void after_load() override;
    void update() override;
};
