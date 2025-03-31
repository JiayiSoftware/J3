#include "hierarchy_system.hpp"

#include "component/ui/hierarchy_layer.hpp"

void hierarchy_system::initialize() {}

void hierarchy_system::update(entt::registry& registry) {
    auto view = registry.view<hierarchy_layer>();
}

void hierarchy_system::destroy() {}
