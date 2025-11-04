#include "main_window.hpp"

#include "framework/component/basic/drawable.hpp"
#include "framework/component/basic/transform.hpp"
#include "framework/system/render/renderer.hpp"
#include "j3/view/sidebar.hpp"

LOAD_RESOURCE(resources_models_jiayi_logo_obj)

main_window::main_window(HINSTANCE instance, const std::wstring& title, vector2 size, bool main_window)
    : window(instance, title, size, main_window) {
    auto& app = application::get();

    this->set_background_color({ 0.058f, 0.058f, 0.058f, 1 });

    // create resources
    std::vector<mesh> jiayi_logo_model = mesh::load(GET_RESOURCE(resources_models_jiayi_logo_obj));
    auto jiayi_logo = app.resources.add<mesh>("jiayi_logo", jiayi_logo_model[0]);

    // jiayi logo
    this->jiayi_logo_entity = this->ecs.create_entity();
    this->ecs.add_component<drawable>(this->jiayi_logo_entity, jiayi_logo);

    auto& tr = this->ecs.add_component<transform>(this->jiayi_logo_entity);
    tr.set_position({ 0, 0, 3 });

    this->rml.register_page<sidebar>();
    this->rml.show_page<sidebar>();
}

void main_window::update() {
    window::update(); // allow ECS to update

    auto& app = application::get();
    auto& t = this->ecs.get_component<transform>(this->jiayi_logo_entity);

    vector3 rotation = t.get_rotation();
    rotation.x += 23.f * app.time.delta_time;
    rotation.y += 20.f * app.time.delta_time;
    rotation.z += 20.f * app.time.delta_time;

    t.set_rotation(rotation);

    // changing rendered content requires an update request to properly show in the window
    this->rml.request_update();
}