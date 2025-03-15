#pragma once
#include "common.hpp"

#include "window.hpp"

class application {
public:
    HINSTANCE instance;
    std::vector<std::unique_ptr<window>> windows;
    bool running = false;
    
    explicit application(HINSTANCE instance);

    static application& get();

    void run();
    void quit();

    std::unique_ptr<window>& create_window(const std::wstring& title, vector2 size);
    std::unique_ptr<window>& create_window(const std::wstring& title, vector2 position, vector2 size);
    std::unique_ptr<window>& get_main_window();
    
private:
    static LRESULT CALLBACK window_proc(HWND handle, UINT message, WPARAM w_param, LPARAM l_param);
};

extern char application_buffer[sizeof(application)];
