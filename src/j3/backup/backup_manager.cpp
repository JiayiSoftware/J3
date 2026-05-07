#include "backup_manager.hpp"

#include "j3/game/minecraft.hpp"

#include <glaze/core/ostream_buffer.hpp>

backup_manager::backup_manager(const std::filesystem::path& path) : current_path(path) {
    spdlog::info("Loading backup collection from {}", path.string());
    auto json_path = this->current_path / "backups.json";
    
    if (std::filesystem::exists(json_path)) {
        if (auto error = glz::read_file_json(this->collection, json_path.string(), std::string{}); !error) {
            spdlog::info("Loaded backup collection");
        } else {
            spdlog::error("Failed to load backup collection: {}", glz::format_error(error));
        }
        
        return;
    }
    
    // create new
    spdlog::debug("Backup collection does not exist here, creating");
    std::filesystem::create_directories(this->current_path);
    if (auto error = glz::write_file_json(this->collection, json_path.string(), std::string{}); !error) {
        spdlog::info("Created new backup collection");
    } else {
        spdlog::error("Failed to create new backup collection: {}", glz::format_error(error));
    }
}

void backup_manager::save() {
    auto json_path = this->current_path / "backups.json";
    
    if (auto error = glz::write_file_json(this->collection, json_path.string(), std::string{}); !error) {
        spdlog::info("Saved backup collection");
    } else {
        spdlog::error("Failed to create new backup collection: {}", glz::format_error(error));
    }
}

backup_collection& backup_manager::get_backups() {
    return this->collection;
}

task backup_manager::create_backup(const std::string& name, const minecraft_version& for_version) {
    return { [=, this](task& t) {
        spdlog::info("Creating {} for version {}", name, for_version.string(false));
        t.name = "Creating backup...";
        
        minecraft game;
        std::filesystem::path game_data = game.data_path();

        std::filesystem::path real_game_data;
        if (!game.is_gdk()) {
            // easy for UWP
            real_game_data = game_data / "LocalState";
            spdlog::debug("Got UWP game platform, game data is at {}", real_game_data.string());
        } else {
            // there is a folder for each Xbox user signed in for GDK and this is very strange
            // so we're backing up the whole Users folder to match UWP behavior
            real_game_data = game_data / "Users";
            spdlog::debug("Got GDK game platform, game data is at {}", real_game_data.string());
        }
        
        t.progress = 0.1f;
        t.name = "Copying game data...";
    
        std::filesystem::path final_backup_path = this->current_path / name;
        std::filesystem::copy(real_game_data, final_backup_path, std::filesystem::copy_options::recursive);
        spdlog::debug("Copied game data to {}", final_backup_path.string());
        
        t.progress = 0.9f;
        t.name = "Saving new backup...";
    
        // new backup
        this->collection.insert(this->collection.begin(), {
            name,
            final_backup_path.string(),
            for_version.string(),
            std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count(),
            this->count_backup_contents(final_backup_path)
        });
    
        spdlog::info("Created {} in {}", name, final_backup_path.string());
        this->save();
        
        t.progress = 1;
    }};
}

void backup_manager::apply_backup(const std::string& name) {
    // not sure how i'm supposed to apply backups across platforms
    spdlog::warn("Not implemented");
}

void backup_manager::rename_backup(const std::string& name, const std::string& new_name) {
    for (auto& backup : this->collection) {
        if (backup.name != name) continue;
        
        backup.name = new_name;
        
        std::filesystem::path new_path = this->current_path / new_name;
        std::filesystem::rename(backup.path, new_path);
        backup.path = new_path.string();
        
        spdlog::info("Renamed {} to {}", name, new_name);
        this->save();
        return;
    }
    
    spdlog::warn("No backup found to rename with the name {}", name);
}

void backup_manager::remove_backup(const std::string& name) {
    for (int i = 0; i < this->collection.size(); i++) {
        if (this->collection[i].name != name) continue;
        
        std::filesystem::remove_all(this->collection[i].path);
        this->collection.erase(this->collection.begin() + i);
        
        spdlog::info("Removed {} from the collection", name);
        this->save();
        return;
    }
    
    spdlog::warn("No backup found to remove with the name {}", name);
}

struct backup::contents backup_manager::count_backup_contents(const std::filesystem::path& path) {
    struct backup::contents contents{ };
    int root_dir_count = 0; // counting com.mojang folders
    
    // lambda for counting top-level folders in a folder
    auto count_directories = [](const std::filesystem::path& dir) {
        if (!std::filesystem::exists(dir)) return 0;
        
        int count = 0;
        for (const auto& _ : std::filesystem::directory_iterator{ dir }) {
            count++;
        }
        
        return count;
    };
    
    for (const auto& dir : std::filesystem::directory_iterator{ path }) {
        if (!dir.is_directory()) continue;
        
        std::filesystem::path real_path = dir.path() / "com.mojang"; // UWP (root is games)
        if (!std::filesystem::exists(real_path)) {
            real_path = dir.path() / "games" / "com.mojang"; // GDK (root is user folder)
            if (!std::filesystem::exists(real_path)) continue;
        }
        
        root_dir_count++;
        
        auto behavior_packs = real_path / "behavior_packs";
        auto resource_packs = real_path / "resource_packs";
        auto worlds = real_path / "minecraftWorlds";
        
        contents.behavior_packs += count_directories(behavior_packs);
        contents.resource_packs += count_directories(resource_packs);
        contents.worlds += count_directories(worlds);
    }
    
    if (root_dir_count == 0) {
        spdlog::warn("Seems this backup was empty, no counts returned");
        return contents;
    }
    
    contents.average = root_dir_count > 1;
    if (contents.average) {
        contents.behavior_packs /= root_dir_count;
        contents.resource_packs /= root_dir_count;
        contents.worlds /= root_dir_count;
    }
    
    spdlog::info("Counted {} behavior packs, {} resource packs, and {} worlds for this backup. This is an {}", 
        contents.behavior_packs, contents.resource_packs, contents.worlds, contents.average ? "average." : "exact count.");
    return contents;
}