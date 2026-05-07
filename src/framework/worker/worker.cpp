#include "worker.hpp"

worker::worker(moodycamel::BlockingConcurrentQueue<task>& task_queue) : task_queue(task_queue) { }

void worker::run() {
    this->running = true;
    
    this->thread = std::thread([&] {
        while (this->running) {
            if (!this->task_queue.wait_dequeue_timed(this->current_task, std::chrono::milliseconds{ 5 })) continue;
            
            spdlog::debug("Task assigned: {}", this->current_task.name);
            this->working = true;
            this->current_task.work(this->current_task);
            this->working = false;
            spdlog::debug("Task completed: {}", this->current_task.name);
            
            if (this->current_task.on_finished)
                this->current_task.on_finished();
        }
    });
}

bool worker::is_working() {
    return this->working;
}

worker::~worker() {
    this->running = false;
    this->thread.join();
}