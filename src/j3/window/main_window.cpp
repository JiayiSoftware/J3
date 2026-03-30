#include "main_window.hpp"

#include "j3/view/backups.hpp"
#include "j3/view/home.hpp"
#include "j3/view/mods.hpp"
#include "j3/view/settings.hpp"
#include "j3/view/sidebar.hpp"
#include "j3/view/test_place.hpp"
#include "j3/view/versions.hpp"

main_window::main_window(HINSTANCE instance, const std::wstring& title, vector2 size, bool main_window)
    : window(instance, title, size, main_window) {
    this->set_background_color({ 0.058f, 0.058f, 0.058f, 1 });

    this->rml.register_page<test_place>();
    
    this->rml.register_page<home>();
    this->rml.register_page<mods>();
    this->rml.register_page<backups>();
    this->rml.register_page<versions>();
    this->rml.register_page<settings>();
    
    this->rml.register_page<sidebar>();

    this->rml.show_page<home>();

    // sidebar must be shown after everything so it appears above everything on first launch
    this->rml.show_page<sidebar>();
}

void main_window::update() {
    window::update(); // allow ECS to update

    // do any updates here
}