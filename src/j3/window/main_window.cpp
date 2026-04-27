#include "main_window.hpp"

#include "j3/view/backups.hpp"
#include "j3/view/home.hpp"
#include "j3/view/mods.hpp"
#include "j3/view/settings.hpp"
#include "j3/view/sidebar.hpp"
#include "j3/view/test_place.hpp"
#include "j3/view/versions.hpp"

main_window::main_window(HINSTANCE instance, const std::wstring& title, const vector2 size, const bool main_window)
    : window(instance, title, size, main_window) {
    this->set_background_color({ 0.058f, 0.058f, 0.058f, 1 });

    this->rml.register_view<test_place>();
    
    this->rml.register_view<home>();
    this->rml.register_view<mods>();
    this->rml.register_view<backups>();
    this->rml.register_view<versions>();
    this->rml.register_view<settings>();
    
    this->rml.register_view<sidebar>();

    this->rml.show_view<home>();

    // sidebar must be shown after everything so it appears above everything on first launch
    this->rml.show_view<sidebar>();
}

void main_window::update() {
    window::update(); // allow ECS to update

    // do any updates here
}