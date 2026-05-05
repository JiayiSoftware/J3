#include "backups.hpp"

void backups::after_load() { }

void backups::update() {
    backups::get_controller().update();
}