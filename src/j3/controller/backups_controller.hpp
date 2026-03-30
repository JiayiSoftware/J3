#pragma once
#include "common.hpp"

#include "framework/ui/controller/controller.hpp"

class backups_controller : public controller {
public:
    void bind_data(Rml::DataModelConstructor& dmc) override;
};
