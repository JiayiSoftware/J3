#pragma once
#include "common.hpp"

#include "framework/ui/controller/controller.hpp"
#include "j3/backup/backup_manager.hpp"
#include "j3/model/backups_model.hpp"

class backups_controller : public controller {
public:
    void bind_data(Rml::DataModelConstructor& dmc) override;
    
private:
    backups_model model;
    backup_manager manager{ "./test_backups" };
    
    void create_backup(Rml::DataModelHandle handle, Rml::Event& e, const Rml::VariantList& args);
};
