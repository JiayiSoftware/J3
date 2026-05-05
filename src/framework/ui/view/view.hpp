#pragma once
#include "common.hpp"
#include "framework/ui/controller/controller.hpp"

template <hat::fixed_string N, std::derived_from<controller> T>
class view {
public:
    static constexpr hat::fixed_string name = N;
    static constexpr hat::fixed_string path = "resources/ui/rml/" + name + ".rml";

    virtual ~view() = default;

    void initialize(Rml::DataModelConstructor& dmc);
    
    virtual void after_load() = 0;
    virtual void update() = 0;

    static T& get_controller() {
        static T instance;
        return instance;
    }
};

template <hat::fixed_string N, std::derived_from<controller> T>
void view<N, T>::initialize(Rml::DataModelConstructor& dmc) {
    view::get_controller().bind_data(dmc);
}
