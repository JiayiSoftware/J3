#pragma once
#include "common.hpp"
#include "j3/version/minecraft_version.hpp"

struct backup {
    struct contents {
        int worlds;
        int resource_packs;
        int behavior_packs;
        bool average; // in the case of GDK versions, which split game data per Xbox account
    };
    
    std::string name;
    std::string path;
    std::string from_version;
    int64_t timestamp;
    contents contents;
};
using backup_collection = std::vector<backup>;

static_assert(glz::reflectable<backup>);
