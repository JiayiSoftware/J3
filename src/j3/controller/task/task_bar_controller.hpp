#pragma once
#include "common.hpp"

#include "framework/ui/controller/controller.hpp"
#include "j3/model/task/task_bar_model.hpp"

class task_bar_controller : public controller {
public:
    void bind_data(Rml::DataModelConstructor& dmc) override;
    
    void update();

private:
    task_bar_model model;
    Rml::DataModelHandle handle;
};
