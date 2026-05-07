#include "backups_controller.hpp"

#include "framework/core/application.hpp"
#include "j3/game/minecraft.hpp"

void backups_controller::bind_data(Rml::DataModelConstructor& dmc) {
    dmc.Bind("current_version", &this->model.current_version);
    dmc.Bind("current_w", &this->model.current_w);
    dmc.Bind("current_rp", &this->model.current_rp);
    dmc.Bind("current_bp", &this->model.current_bp);
    
    dmc.RegisterTransformFunc("format_timestamp", [&](const Rml::VariantList& args) -> Rml::Variant {
        if (args.empty()) return { };
        auto backup_name = args[0].Get<Rml::String>();
        
        int64_t t1{ 0 };
        int64_t t2{ 0 };

        backup_collection& backups = this->manager.get_backups();
        for (int i = 0; i < backups.size(); ++i) {
            if (backups[i].name != backup_name) continue;
            
            t2 = backups[i].timestamp;
            t1 = i == 0 
                ? std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count()
                : backups[i - 1].timestamp;
            
            break;
        }
        
        std::chrono::seconds seconds{ t1 - t2 };
        
        Rml::SystemInterface* system = Rml::GetSystemInterface();
        Rml::String ret;

        if (const std::chrono::years years = std::chrono::duration_cast<std::chrono::years>(seconds);
            years.count() > 0) {
            system->TranslateString(ret, "general.time.long_time_earlier");
            return Rml::Variant{ ret };
        }

        if (const std::chrono::weeks weeks = std::chrono::duration_cast<std::chrono::weeks>(seconds);
            weeks.count() > 0) {
            system->TranslateString(ret, "general.time.weeks_earlier");
            return Rml::Variant{ fmt::format(fmt::runtime(ret), weeks.count()) };
        }

        if (const std::chrono::days days = std::chrono::duration_cast<std::chrono::days>(seconds); days.count() > 0) {
            system->TranslateString(ret, "general.time.days_earlier");
            return Rml::Variant{ fmt::format(fmt::runtime(ret), days.count()) };
        }

        if (const std::chrono::hours hours = std::chrono::duration_cast<std::chrono::hours>(seconds);
            hours.count() > 0) {
            system->TranslateString(ret, "general.time.hours_earlier");
            return Rml::Variant{ fmt::format(fmt::runtime(ret), hours.count()) };
        }
        
        if (const std::chrono::minutes minutes = std::chrono::duration_cast<std::chrono::minutes>(seconds);
            minutes.count() > 1) {
            system->TranslateString(ret, "general.time.minutes_earlier");
            return Rml::Variant{ fmt::format(fmt::runtime(ret), minutes.count()) };
        }
        
        system->TranslateString(ret, "general.time.moments_earlier");
        return Rml::Variant{ ret };
    });
    
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
    
    if (auto backup_manager_handle = dmc.RegisterStruct<backup_manager>()) {
        backup_manager_handle.RegisterMember("collection", &backup_manager::get_backups);
    }
    
    dmc.Bind("manager", &this->manager);
    
    dmc.BindEventCallback("create_backup", &backups_controller::create_backup, this);
    dmc.BindEventCallback("apply_backup", &backups_controller::apply_backup, this);
    dmc.BindEventCallback("rename_backup", &backups_controller::rename_backup, this);
    dmc.BindEventCallback("delete_backup", &backups_controller::delete_backup, this);
    
    this->handle = dmc.GetModelHandle();
    
    // init values
    minecraft game;
    this->model.current_version = game.version().string();
    
    auto current_contents = this->manager.count_backup_contents(game.data_path());
    this->model.current_w = current_contents.worlds;
    this->model.current_rp = current_contents.resource_packs;
    this->model.current_bp = current_contents.behavior_packs;
}

void backups_controller::update() {
    if (this->needs_update) {
        this->needs_update = false;
        
        this->handle.DirtyVariable("manager");
        this->create_button->RemoveAttribute("disabled");
    }
}

void backups_controller::create_backup(Rml::DataModelHandle handle, Rml::Event& e, const Rml::VariantList& args) {
    this->create_button = e.GetTargetElement();
    
    if (this->create_button->HasAttribute("disabled")) return;
    this->create_button->SetAttribute("disabled", true);
    
    task t = this->manager.create_backup(
        std::format("Backup {}", this->manager.get_backups().size() + 1),
        this->model.current_version
    );
    
    t.on_finished = [&] {
        this->needs_update = true;
    };
    
    auto& app = application::get();
    app.workers.add_task(t);
}

void backups_controller::apply_backup(Rml::DataModelHandle handle, Rml::Event& e, const Rml::VariantList& args) {
    if (args.empty()) return;
    auto backup_name = args[0].Get<Rml::String>();
    
    this->manager.apply_backup(backup_name);
    handle.DirtyVariable("manager");
}

void backups_controller::rename_backup(Rml::DataModelHandle handle, Rml::Event& e, const Rml::VariantList& args) {
    
}

void backups_controller::delete_backup(Rml::DataModelHandle handle, Rml::Event& e, const Rml::VariantList& args) {
    if (args.empty()) return;
    auto backup_name = args[0].Get<Rml::String>();
    
    this->manager.remove_backup(backup_name);
    handle.DirtyVariable("manager");
}