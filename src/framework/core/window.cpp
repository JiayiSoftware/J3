#include "window.hpp"

#include "application.hpp"
#include "framework/component/basic/camera.hpp"
#include "framework/component/ui/rml_container.hpp"
#include "framework/system/render/renderer.hpp"

window::window(const HINSTANCE instance, const std::wstring& title, const vector2 size, const bool main_window) {
    this->main_window = main_window;
    this->finish_create(instance, title, { CW_USEDEFAULT, CW_USEDEFAULT }, size);
}

window::window(
    const HINSTANCE instance, const std::wstring& title, const vector2 position, const vector2 size,
    const bool main_window
) {
    this->main_window = main_window;
    this->finish_create(instance, title, position, size);
}

void window::finish_create(
    const HINSTANCE instance, const std::wstring& title, const vector2 position, const vector2 size
) {
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    RECT rect{ 0, 0, static_cast<LONG>(size.x), static_cast<LONG>(size.y) };
    AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_APPWINDOW);

    const vector2 real_size{ static_cast<float>(rect.right - rect.left), static_cast<float>(rect.bottom - rect.top) };

    this->handle = CreateWindowEx(
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

    if (this->handle == nullptr) {
        spdlog::error("Window creation failed with result 0x{:08X}", GetLastError());
        return;
    }

    spdlog::debug("Window created");

    // set theme awareness here i guess
    BOOL value = true;
    HRESULT hr = DwmSetWindowAttribute(this->handle, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
    LOG_HRESULT(error, "Failed to set theme awareness for window", hr);

    // add and initialize systems
    auto& r = this->ecs.add_system<renderer>(handle, size); // hardware accelerated by default

    // add camera
    this->ecs.add_component<camera>(this->ecs.create_entity());
    
    // init and hand rml over to ecs so the renderer can access it
    this->rml.initialize(this->handle, size, r.get_device(), r.get_rtv());
    ecs.add_component<rml_container>(this->ecs.create_entity(), this->rml);

    this->rml.set_dip_ratio(this->get_dip_ratio());

    spdlog::debug("Window systems initialized");
}

void window::show() const {
    ShowWindow(this->handle, SW_SHOW);
    SetForegroundWindow(this->handle);
    SetFocus(this->handle);

    spdlog::debug("Window shown");
}

void window::update() {
    this->ecs.update();
}

void window::close() {
    this->rml.destroy();

    this->closing = true;
    DestroyWindow(this->handle);

    auto& app = application::get();
    if (this->main_window) {
        spdlog::debug("Main window closed");
        app.windows.clear();
        app.quit();
        return; // although app terminates at this point
    }

    spdlog::debug("Window closed");

    std::erase_if(app.windows, [this](const std::unique_ptr<window>& window) {
        return window->handle == this->handle;
    });
}

bool window::get_focused() const { return GetFocus() == this->handle; }

float window::get_dip_ratio() const {
    UINT dpi = GetDpiForWindow(this->handle);
    if (dpi == 0) dpi = USER_DEFAULT_SCREEN_DPI;
    
    return static_cast<float>(dpi) / static_cast<float>(USER_DEFAULT_SCREEN_DPI);
}

void window::set_background_color(const vector4& color) const {
    renderer& rend = renderer::get_for_window(this->handle);
    rend.set_background_color(color);
}

bool window::window_proc(const UINT message, const WPARAM w_param, const LPARAM l_param) {
    if (message == WM_CLOSE) {
        close();
        return true;
    }

    if (message == WM_SIZE) {
        UINT width = LOWORD(l_param);
        UINT height = HIWORD(l_param);

        // do you guys like back and forth conversion
        vector2 new_size = { static_cast<float>(width), static_cast<float>(height) };

        renderer& rend = renderer::get_for_window(this->handle);
        rend.resize(new_size);
        this->rml.resize(new_size, rend.get_rtv());
        
        return true;
    }

    if (message == WM_SIZING) {
        auto rect = reinterpret_cast<RECT*>(l_param);
        UINT width = rect->right - rect->left;
        UINT height = rect->bottom - rect->top;

        vector2 new_size = { static_cast<float>(width), static_cast<float>(height) };

        renderer& rend = renderer::get_for_window(this->handle);
        rend.resize(new_size);
        this->rml.resize(new_size, rend.get_rtv());
        
        return true;
    }
    
    if (message == WM_DPICHANGED) {
        auto new_rect = reinterpret_cast<RECT*>(l_param);
        UINT width = new_rect->right - new_rect->left;
        UINT height = new_rect->bottom - new_rect->top;
        
        SetWindowPos(
            this->handle,
            nullptr,
            new_rect->left,
            new_rect->top,
            width,
            height,
            SWP_NOZORDER | SWP_NOACTIVATE
        );
        
        this->rml.set_dip_ratio(this->get_dip_ratio());

        vector2 new_size = { static_cast<float>(width), static_cast<float>(height) };

        renderer& rend = renderer::get_for_window(this->handle);
        rend.resize(new_size);
        this->rml.resize(new_size, rend.get_rtv());
        
        return true;
    }

    if (this->rml.window_procedure(this->handle, message, w_param, l_param)) return true;

    return false; // not handled
}

window::~window() {
    if (!closing) window::close();
}
