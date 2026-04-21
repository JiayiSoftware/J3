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
        spdlog::info("Created new backup collection");
    } else {
        spdlog::error("Failed to create new backup collection: {}", glz::format_error(error));
    }
}

backup_collection& backup_manager::get_backups() {
    return this->collection;
}

void backup_manager::create_backup(const std::string& name, const minecraft_version& for_version) {
    spdlog::info("Creating {} for version {}", name, for_version);
    
    minecraft game;
    std::filesystem::path game_data = game.data_path();

    std::filesystem::path real_game_data;
    if (!game.is_gdk()) {
        // easy for UWP
        real_game_data = game_data / "LocalState" / "games" ;
        spdlog::debug("Got UWP game platform, game data is at {}", real_game_data.string());
    } else {
        // there is a folder for each Xbox user signed in for GDK and this is very strange
        // so we're backing up the whole Users folder to match UWP behavior
        real_game_data = game_data / "Users";
        spdlog::debug("Got GDK game platform, game data is at {}", real_game_data.string());
    }
    
    std::filesystem::path final_backup_path = this->current_path / name;
    std::filesystem::copy(real_game_data, final_backup_path, std::filesystem::copy_options::recursive);
    spdlog::debug("Copied game data to {}", final_backup_path.string());
    
    // new backup
    this->collection.push_back({
        name,
        final_backup_path.string(),
        for_version,
        std::chrono::system_clock::now().time_since_epoch().count(),
        this->count_backup_contents(final_backup_path)
    });
    
    spdlog::info("Created {} in {}", name, final_backup_path.string());
    this->save();
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
        std::ranges::for_each(std::filesystem::directory_iterator{dir}, [&](const auto& _) {
            count++;
        });
        
        return count;
    };
    
    std::ranges::for_each(std::filesystem::recursive_directory_iterator{path}, [&](const auto& dir) {
        if (dir.path().filename().string() != "com.mojang") return;
        
        root_dir_count++;
        
        auto behavior_packs = dir.path() / "behavior_packs";
        auto resource_packs = dir.path() / "resource_packs";
        auto worlds = dir.path() / "minecraftWorlds";
        
        contents.behavior_packs += count_directories(behavior_packs);
        contents.resource_packs += count_directories(resource_packs);
        contents.worlds += count_directories(worlds);
    });
    
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