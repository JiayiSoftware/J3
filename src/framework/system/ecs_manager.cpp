#include "ecs_manager.hpp"

void ecs_manager::update() {
    for (auto& system : this->systems) {
        system->update(registry);
    }
}

entt::entity ecs_manager::create_entity() {
    return this->registry.create();
}
