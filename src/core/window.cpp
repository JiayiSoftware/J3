#include "window.hpp"

#include "application.hpp" // avoid circular dependency

window::window(const HINSTANCE instance, const std::wstring& title, const vector2 size, const bool main_window) {
    this->main_window = main_window;
    this->finish_create(instance, title, {CW_USEDEFAULT, CW_USEDEFAULT}, size);
}

window::window(const HINSTANCE instance, const std::wstring& title, const vector2 position, const vector2 size, const bool main_window) {
    this->main_window = main_window;
    this->finish_create(instance, title, position, size);
}

void window::finish_create(const HINSTANCE instance, const std::wstring& title, const vector2 position, const vector2 size) {
    RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = size.x;
    rect.bottom = size.y;

    AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_APPWINDOW);

    vector2 real_size = { static_cast<float>(rect.right - rect.left), static_cast<float>(rect.bottom - rect.top) };
    
    handle = CreateWindowEx(
        WS_EX_APPWINDOW,
        WINDOW_CLASS_NAME,
        title.c_str(),
        WS_OVERLAPPEDWINDOW,
        position.x,
        position.y,
        real_size.x,
        real_size.y,
        nullptr,
        nullptr,
        instance,
        nullptr
    );

    if (handle == nullptr) {
        // handle error
    }

    renderer.initialize(handle, size); // hardware accelerated by default
}

void window::show() const {
    ShowWindow(handle, SW_SHOW);
    UpdateWindow(handle);
}

void window::update() {
    renderer.render_frame();
}

void window::close() {
    closing = true;
    DestroyWindow(handle);
    
    auto& app = application::get();
    if (main_window) {
        app.windows.clear();
        app.quit();
        return; // although app terminates at this point
    }
    
    std::erase_if(app.windows, [this](const std::unique_ptr<window>& window) {
        return window->handle == this->handle;
    });
}

bool window::window_proc(UINT message, WPARAM w_param, LPARAM l_param) {
    // just handle quit for now
    if (message == WM_CLOSE) {
        close();
        return true;
    }

    return false; // not handled
}

window::~window() {
    if (!closing) close();
}
