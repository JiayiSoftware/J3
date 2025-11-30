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
    auto& app = application::get();

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
        app.log.error("Window creation failed with result 0x{:08X}", GetLastError());
        return;
    }

    app.log.debug("Window created");

    // set theme awareness here i guess
    BOOL value = true;
    HRESULT hr = DwmSetWindowAttribute(this->handle, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
    LOG_HRESULT(error, "Failed to set theme awareness for window", hr);

    // add and initialize systems
    auto& r = this->ecs.add_system<renderer>(handle, size); // hardware accelerated by default

    // add camera
    auto camera_entity = this->ecs.create_entity();
    this->ecs.add_component<camera>(camera_entity);

    // initialize services
    this->rml.initialize(this->handle, size, r.get_device(), r.get_rtv());

    // hand rml over to ecs so the renderer can access it
    auto rml_entity = this->ecs.create_entity();
    ecs.add_component<rml_container>(rml_entity, this->rml);

    app.log.debug("Window systems initialized");
}

void window::show() const {
    ShowWindow(this->handle, SW_SHOW);
    SetForegroundWindow(this->handle);
    SetFocus(this->handle);

    application::get().log.debug("Window shown");
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
        app.log.debug("Main window closed");
        app.windows.clear();
        app.quit();
        return; // although app terminates at this point
    }

    app.log.debug("Window closed");

    std::erase_if(app.windows, [this](const std::unique_ptr<window>& window) {
        return window->handle == this->handle;
    });
}

bool window::get_focused() const { return GetFocus() == this->handle; }

void window::set_background_color(const vector4& color) const {
    renderer& rend = renderer::get_for_window(this->handle);
    rend.set_background_color(color);
}

bool window::window_proc(UINT message, WPARAM w_param, LPARAM l_param) {
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
    }

    if (message == WM_SIZING) {
        auto rect = reinterpret_cast<RECT*>(l_param);
        UINT width = rect->right - rect->left;
        UINT height = rect->bottom - rect->top;

        vector2 new_size = { static_cast<float>(width), static_cast<float>(height) };

        renderer& rend = renderer::get_for_window(this->handle);
        rend.resize(new_size);
        this->rml.resize(new_size, rend.get_rtv());
    }

    if (this->rml.window_procedure(this->handle, message, w_param, l_param)) return true;

    return false; // not handled
}

window::~window() {
    if (!closing) window::close();
}
