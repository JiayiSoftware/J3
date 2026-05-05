#include "sidebar.hpp"

void sidebar::after_load() { }

void sidebar::update() {
    sidebar::get_controller().update();
}