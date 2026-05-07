#pragma once
#include "common.hpp"

#include "task.hpp"

#include <blockingconcurrentqueue.h>

class worker {
public:
    std::atomic<bool> running{ false };
    task current_task{ };
    
    worker(moodycamel::BlockingConcurrentQueue<task>& task_queue);
    ~worker(); // My only purpose is to work. If I am not working, I am as good as dead.
    
    void run();
    bool is_working();
    
private:
    std::thread thread{};
    moodycamel::BlockingConcurrentQueue<task>& task_queue;
    bool working{ false };
};
