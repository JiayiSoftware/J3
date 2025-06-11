#include "window.hpp"

#include "application.hpp"
#include "component/basic/camera.hpp"
#include "component/basic/drawable.hpp"
#include "component/basic/transform.hpp"
#include "component/ui/rml_container.hpp"
#include "system/render/renderer.hpp"
#include "ui/pages/test_place.hpp"

LOAD_RESOURCE(resources_textures_mart_png)
LOAD_RESOURCE(resources_models_jiayi_logo_obj)

window::window(const HINSTANCE instance, const std::wstring& title, const vector2 size, const bool main_window) {
    this->main_window = main_window;
    this->finish_create(instance, title, {CW_USEDEFAULT, CW_USEDEFAULT}, size);
}

window::window(const HINSTANCE instance, const std::wstring& title, const vector2 position, const vector2 size, const bool main_window) {
    this->main_window = main_window;
    this->finish_create(instance, title, position, size);
}

void window::finish_create(const HINSTANCE instance, const std::wstring& title, const vector2 position, const vector2 size) {
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

    // add and initialize systems
    auto& r = this->ecs.add_system<renderer>(handle, size); // hardware accelerated by default
    this->ecs.initialize();

    this->set_background_color({ 0.058f, 0.058f, 0.058f, 1 });

    // add camera
    auto camera_entity = this->ecs.create_entity();
    this->ecs.add_component<camera>(camera_entity);

    // create resources

    std::vector<mesh> jiayi_logo_model = mesh::load(GET_RESOURCE(resources_models_jiayi_logo_obj));
    auto jiayi_logo = app.resources.add<mesh>("jiayi_logo", jiayi_logo_model[0]);

    // initialize services
    this->rml.initialize(this->handle, size, r.get_device(), r.get_rtv());
    this->rml.register_page<test_place>();
    this->rml.show_page<test_place>();

    // hand rml over to ecs so the renderer can access it
    auto rml_entity = this->ecs.create_entity();
    ecs.add_component<rml_container>(rml_entity, this->rml);
    
    // test drawing entity
    this->jiayi_logo_entity = this->ecs.create_entity();
    ecs.add_component<drawable>(this->jiayi_logo_entity, jiayi_logo);

    auto& tr = this->ecs.add_component<transform>(this->jiayi_logo_entity);
    tr.set_position({ 0, 0, 3 });

    app.log.debug("Window systems initialized");
}

void window::show() const {
    ShowWindow(this->handle, SW_SHOW);
    UpdateWindow(this->handle);
    
    application::get().log.debug("Window shown");
}

void window::update() {
    this->ecs.update();

    auto& t = this->ecs.get_component<transform>(this->jiayi_logo_entity);
    
    vector3 rotation = t.get_rotation();
    rotation.x += 0.53f;
    rotation.y += 0.5f;
    rotation.z += 0.5f;

    t.set_rotation(rotation);
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
    
    if (this->rml.window_procedure(this->handle, message, w_param, l_param))
        return true;

    return false; // not handled
}

window::~window() {
    if (!closing) close();
}
