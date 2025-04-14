#pragma once
#include "common.hpp"

#include "resource/resource_manager.hpp"
#include "log.hpp"
#include "timer.hpp"
#include "window.hpp"

class application {
public:
    std::vector<std::unique_ptr<window>> windows;
    timer time;
    resource_manager resources;
    class log log;
    
    explicit application(HINSTANCE instance);

    static application& get();

    void run();
    void quit(int exit_code = 0);

    std::unique_ptr<window>& create_window(const std::wstring& title, vector2 size);
    std::unique_ptr<window>& create_window(const std::wstring& title, vector2 position, vector2 size);
    std::unique_ptr<window>& get_main_window();
    
private:
    HINSTANCE instance;
    bool running = false;
    
    static LRESULT CALLBACK window_proc(HWND handle, UINT message, WPARAM w_param, LPARAM l_param);
};

extern char application_buffer[sizeof(application)];
