#include "backups_controller.hpp"

#include "framework/core/application.hpp"
#include "j3/game/minecraft.hpp"

void backups_controller::bind_data(Rml::DataModelConstructor& dmc) {
    dmc.Bind("current_version", &this->model.current_version);
    dmc.Bind("current_w", &this->model.current_w);
    dmc.Bind("current_rp", &this->model.current_rp);
    dmc.Bind("current_bp", &this->model.current_bp);
    
    if (auto backup_contents_handle = dmc.RegisterStruct<struct backup::contents>()) {
        backup_contents_handle.RegisterMember("worlds", &backup::contents::worlds);
        backup_contents_handle.RegisterMember("resource_packs", &backup::contents::resource_packs);
        backup_contents_handle.RegisterMember("behavior_packs", &backup::contents::behavior_packs);
        backup_contents_handle.RegisterMember("average", &backup::contents::average);
    }
    
    if (auto backup_handle = dmc.RegisterStruct<backup>()) {
        backup_handle.RegisterMember("name", &backup::name);
        backup_handle.RegisterMember("from_version", &backup::from_version);
        backup_handle.RegisterMember("timestamp", &backup::timestamp);
        backup_handle.RegisterMember("contents", &backup::contents);
    }
    
    dmc.RegisterArray<backup_collection>();
    dmc.Bind("collection", &this->model.collection);
    
    dmc.BindEventCallback("create_backup", &backups_controller::create_backup, this);
    dmc.BindEventCallback("apply_backup", &backups_controller::apply_backup, this);
    dmc.BindEventCallback("rename_backup", &backups_controller::rename_backup, this);
    dmc.BindEventCallback("delete_backup", &backups_controller::delete_backup, this);
    
    // init values
    minecraft game;
    this->model.current_version = game.version().string();
    
    auto current_contents = this->manager.count_backup_contents(game.data_path());
    this->model.current_w = current_contents.worlds;
    this->model.current_rp = current_contents.resource_packs;
    this->model.current_bp = current_contents.behavior_packs;
    
    this->model.collection = this->manager.get_backups();
}

void backups_controller::update() {
    
}

void backups_controller::create_backup(Rml::DataModelHandle handle, Rml::Event& e, const Rml::VariantList& args) {
    task t = this->manager.create_backup(
        std::format("Backup {}", this->manager.get_backups().size() + 1),
        this->model.current_version
    );
    
    auto& app = application::get();
    app.workers.add_task(t);
    
    handle.DirtyVariable("collection");
}

void backups_controller::apply_backup(Rml::DataModelHandle handle, Rml::Event& e, const Rml::VariantList& args) {
    if (args.empty()) return;
    auto backup_name = args[0].Get<Rml::String>();
    
    this->manager.apply_backup(backup_name);
    handle.DirtyVariable("collection");
}

void backups_controller::rename_backup(Rml::DataModelHandle handle, Rml::Event& e, const Rml::VariantList& args) {
    
}

void backups_controller::delete_backup(Rml::DataModelHandle handle, Rml::Event& e, const Rml::VariantList& args) {
    if (args.empty()) return;
    auto backup_name = args[0].Get<Rml::String>();
    
    this->manager.remove_backup(backup_name);
    handle.DirtyVariable("collection");
}