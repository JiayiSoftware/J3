#pragma once
#include "common.hpp"

#include "framework/system/ecs_manager.hpp"
#include "framework/ui/rml_system.hpp"

class window {
public:
    HWND handle;
    bool main_window;
    ecs_manager ecs;
    rml_system rml;

    window(HINSTANCE instance, const std::wstring& title, vector2 size, bool main_window);
    window(HINSTANCE instance, const std::wstring& title, vector2 position, vector2 size, bool main_window);

    void finish_create(HINSTANCE instance, const std::wstring& title, vector2 position, vector2 size);

    void show() const;

    virtual void update();
    virtual void close();

    bool get_focused() const;
    
    void set_background_color(const vector4& color) const;
    
    bool window_proc(UINT message, WPARAM w_param, LPARAM l_param);

    virtual ~window();

private:
    bool closing;
};
