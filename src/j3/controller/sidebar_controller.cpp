#include "sidebar_controller.hpp"

#include "framework/core/application.hpp"
#include "j3/view/backups.hpp"
#include "j3/view/home.hpp"
#include "j3/view/mods.hpp"
#include "j3/view/settings.hpp"
#include "j3/view/task/task_bar.hpp"
#include "j3/view/versions.hpp"

void sidebar_controller::bind_data(Rml::DataModelConstructor& dmc) {
    dmc.Bind("selected_tab", &this->model.selected_tab);
    dmc.Bind("pinned", &this->model.pinned);

    dmc.BindEventCallback("switch_tab", &sidebar_controller::switch_tab, this);
}

void sidebar_controller::update() {
    auto& app = application::get();
    const auto& window = app.get_main_window();
    
    // always update task bar
    window->rml.update_view<task_bar>();
    
    switch (this->model.selected_tab) {
    case 0:
        window->rml.update_view<home>();
        break;
    case 1:
        window->rml.update_view<mods>();
        break;
    case 2:
        window->rml.update_view<backups>();
        break;
    case 3:
        window->rml.update_view<versions>();
        break;
    case 4:
        window->rml.update_view<settings>();
        break;
    default:
        spdlog::warn("Attempted to update unknown view: {}", this->model.selected_tab);
        break;
    }
}

void sidebar_controller::switch_tab(Rml::DataModelHandle handle, Rml::Event& event, const Rml::VariantList& args) {
    if (args.empty()) return;
    
    int requested_tab = args[0].Get(-1);
    if (requested_tab == -1) return;
    this->model.selected_tab = requested_tab;
    handle.DirtyVariable("selected_tab");
    
    auto& app = application::get();
    const auto& window = app.get_main_window();
    
    window->rml.hide_view<home>();
    window->rml.hide_view<mods>();
    window->rml.hide_view<backups>();
    window->rml.hide_view<versions>();
    window->rml.hide_view<settings>();
    
    switch (requested_tab) {
    case 0:
        window->rml.show_view<home>();
        break;
    case 1:
        window->rml.show_view<mods>();
        break;
    case 2:
        window->rml.show_view<backups>();
        break;
    case 3:
        window->rml.show_view<versions>();
        break;
    case 4:
        window->rml.show_view<settings>();
        break;
    default:
        spdlog::warn("Unknown view requested: {}", requested_tab);
        break;
    }
    
    // make sure sidebar is on top after switching tabs
    event.GetTargetElement()->GetOwnerDocument()->PullToFront();
}