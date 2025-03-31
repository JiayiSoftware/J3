#include "window.hpp"

#include "application.hpp" // avoid circular dependency
#include "component/basic/drawable.hpp"
#include "component/basic/transform.hpp"
#include "system/render/renderer.hpp"

LOAD_RESOURCE(resources_textures_mart_png)

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

    // add and initialize systems
    ecs.add_system<renderer>(handle, size); // hardware accelerated by default
    ecs.initialize();

    // create resources
    auto& app = application::get();
    
    auto quad = app.resources.add<mesh>("quad", std::vector<vertex>{
        { { -0.5f, -0.5f }, { 0.0f, 0.0f, 0.0f, 0.5f }, { 0.0f, 0.0f } },
        { { 0.5f, -0.5f }, { 0.0f, 0.0f, 0.0f, 0.5f }, { 1.0f, 0.0f } },
        { { -0.5f, 0.5f }, { 0.0f, 0.0f, 0.0f, 0.5f }, { 0.0f, 1.0f } },
        { { 0.5f, 0.5f }, { 0.0f, 0.0f, 0.0f, 0.5f }, { 1.0f, 1.0f } },
    }, std::vector<DWORD>{
        0, 1, 2,
        2, 1, 3
    });

    auto mart = app.resources.add<texture>("mart", GET_RESOURCE(resources_textures_mart_png));

    auto red_quad = app.resources.add<mesh>("red_quad", std::vector<vertex>{
        { { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
        { { 0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } },
        { { -0.5f, 0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
        { { 0.5f, 0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
    }, std::vector<DWORD>{
        0, 1, 2,
        2, 1, 3
    });

    // test drawing entity
    auto entity = ecs.create_entity();
    ecs.add_component<drawable>(entity, quad, mart);

    auto& tr = ecs.add_component<transform>(entity);
    tr.set_scale({200, 200});

    auto entity2 = ecs.create_entity();
    auto& d = ecs.add_component<drawable>(entity2);
    d.mesh = red_quad;

    auto& tr2 = ecs.add_component<transform>(entity2);
    tr2.set_position({100, 100});
    tr2.set_scale({100, 100});
}

void window::show() const {
    ShowWindow(handle, SW_SHOW);
    UpdateWindow(handle);
}

void window::update() {
    ecs.update();
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
