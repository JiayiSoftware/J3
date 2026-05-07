#include "task_bar_controller.hpp"

#include "framework/core/application.hpp"
#include "framework/worker/worker_pool.hpp"

void task_bar_controller::bind_data(Rml::DataModelConstructor& dmc) {
    dmc.Bind("task_status", &this->model.task_status);
    dmc.Bind("total_progress", &this->model.total_progress);
    dmc.Bind("shown", &this->model.shown);
    
    this->handle = dmc.GetModelHandle();
}

void task_bar_controller::update() {
    const auto& app = application::get();
    Rml::SystemInterface* system = Rml::GetSystemInterface();
    
    int current_tasks_count{ 0 };
    float wish_total_progress{ 0.0f };
    Rml::String wish_task_status;
    
    for (const auto& worker : app.workers.workers) {
        if (!worker->is_working()) continue;
        
        current_tasks_count++;
        wish_total_progress += worker->current_task.progress;
        wish_task_status = worker->current_task.name;
    }
    
    if (current_tasks_count == 0) {
        this->model.shown = false;
        this->handle.DirtyVariable("shown");
        return;
    }
    
    this->model.shown = true;
    if (current_tasks_count > 1) {
        wish_total_progress /= current_tasks_count;
        
        system->TranslateString(wish_task_status, "general.task.multiple");
        wish_task_status = fmt::format(fmt::runtime(wish_task_status), current_tasks_count);
    }
    
    this->model.task_status = wish_task_status;
    this->model.total_progress = wish_total_progress;
    this->handle.DirtyAllVariables(); // all variables are dirty at this point
}