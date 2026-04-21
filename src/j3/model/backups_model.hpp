#pragma once
#include "common.hpp"

#include "j3/backup/backup.hpp"

struct backups_model {
    Rml::String current_version{ "Unknown" };
    int current_w{ 0 };
    int current_rp{ 0 };
    int current_bp{ 0 };
    backup_collection collection;
};
