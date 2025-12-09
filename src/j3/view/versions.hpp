#pragma once
#include "common.hpp"

#include "framework/ui/view/view.hpp"
#include "j3/controller/versions_controller.hpp"

class versions : public view<"versions", versions_controller> {
public:
    void after_load() override;
};
