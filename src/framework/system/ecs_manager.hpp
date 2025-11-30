#pragma once
#include "common.hpp"
#include "entity_system.hpp"

class ecs_manager {
public:
    void update();

    [[nodiscard]] entt::entity create_entity();

    template <typename component_t, typename... args>
    component_t& add_component(entt::entity entity, args&&... a);

    template <typename component_t>
    component_t& get_component(entt::entity entity);

    template <typename system_t, typename... args>
    system_t& add_system(args&&... a);

private:
    std::vector<entity_system> systems;
    entt::registry registry;
};

template <typename component_t, typename ... args>
component_t& ecs_manager::add_component(entt::entity entity, args&&... a) {
    return this->registry.emplace<component_t>(entity, std::forward<args>(a)...);
}

template <typename component_t>
component_t& ecs_manager::get_component(const entt::entity entity) {
    return this->registry.get<component_t>(entity);
}

template <typename system_t, typename ... args>
system_t& ecs_manager::add_system(args&&... a) {
    static system_t system(std::forward<args>(a)...);
    this->systems.push_back(entity_system{std::in_place_type<system_t&>, system});
    system.initialize();
    return system;
}
