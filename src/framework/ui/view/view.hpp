#pragma once
#include "common.hpp"
#include "framework/ui/controller/controller.hpp"

template <std::derived_from<controller> T>
class view {
public:
    virtual ~view() = default;

    void initialize(Rml::DataModelConstructor& dmc);
    virtual void after_load() = 0;

private:
    static T& get_controller() {
        static T instance;
        return instance;
    }
};

template <std::derived_from<controller> T>
void view<T>::initialize(Rml::DataModelConstructor& dmc) {
    get_controller().bind_data(dmc);
}
