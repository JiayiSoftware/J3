#pragma once
#include "common.hpp"

#include "framework/ui/view/view.hpp"
#include "j3/controller/archives_controller.hpp"

class archives : public view<"archives", archives_controller> {
public:
    void after_load() override;
};
