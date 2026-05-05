#pragma once
#include "common.hpp"
#include "framework/core/special_folder.hpp"

#include "framework/ui/controller/controller.hpp"
#include "j3/backup/backup_manager.hpp"
#include "j3/model/backups_model.hpp"

class backups_controller : public controller {
public:
    void bind_data(Rml::DataModelConstructor& dmc) override;
    
    void update();
    
private:
    backups_model model;
    backup_manager manager{ special_folder::get(FOLDERID_LocalAppData) / "J3" / "Backups" };
    
    void create_backup(Rml::DataModelHandle handle, Rml::Event& e, const Rml::VariantList& args);
    void apply_backup(Rml::DataModelHandle handle, Rml::Event& e, const Rml::VariantList& args);
    void rename_backup(Rml::DataModelHandle handle, Rml::Event& e, const Rml::VariantList& args);
    void delete_backup(Rml::DataModelHandle handle, Rml::Event& e, const Rml::VariantList& args);
};
