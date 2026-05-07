#include "task_bar.hpp"

void task_bar::after_load() {
    
}

void task_bar::update() {
    task_bar::get_controller().update();
}