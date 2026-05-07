#pragma once
#include "common.hpp"

#include "framework/ui/view/view.hpp"
#include "j3/controller/task/task_bar_controller.hpp"

class task_bar : public view<"task/task_bar", task_bar_controller> {
public:
    void after_load() override;
    void update() override;
};
