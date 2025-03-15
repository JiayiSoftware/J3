#include "application.hpp"

alignas(application) char application_buffer[sizeof(application)];

application::application(const HINSTANCE instance) {
    this->instance = instance;

    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) { // although this will probably never fail
        // handle error
    }
    
    // get icon because i don't want to load one from a file
    const auto module_path = new wchar_t[MAX_PATH];
    GetModuleFileName(this->instance, module_path, MAX_PATH);
    HICON icon = ExtractIcon(this->instance, module_path, 0);

    // application owns the window class
    WNDCLASSEXW window_class = {};
    window_class.cbSize = sizeof(WNDCLASSEXW);
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = window_proc;
    window_class.hInstance = this->instance;
    window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
    window_class.hbrBackground = nullptr;
    window_class.hIcon = icon;
    window_class.lpszClassName = WINDOW_CLASS_NAME;

    ATOM atom = RegisterClassExW(&window_class);
    if (atom == 0) {
        // handle error
    }

    // clean up
    delete[] module_path;
}

application& application::get() {
    return *std::launder(reinterpret_cast<application*>(application_buffer));
}

void application::run() {
    if (this->windows.empty()) {
        // handle error
    }

    running = true;

    const auto& main_window = get_main_window();
    main_window->show();

    // window loop
    MSG message = { };
    while (running) {
        while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessage(&message);

            if (message.message == WM_QUIT) {
                running = false;
                break;
            }
        }

        // perform updates during dead time
        for (const auto& window : windows) {
            window->update();
        }

        // TODO: run a timer before updating
    }
}

void application::quit() {
    PostQuitMessage(0);
}

std::unique_ptr<window>& application::create_window(const std::wstring& title, vector2 size) {
    this->windows.push_back(std::make_unique<window>(this->instance, title, size, this->windows.empty()));
    return this->windows.back();
}

std::unique_ptr<window>& application::create_window(const std::wstring& title, vector2 position, vector2 size) {
    this->windows.push_back(std::make_unique<window>(this->instance, title, position, size, this->windows.empty()));
    return this->windows.back();
}

std::unique_ptr<window>& application::get_main_window() {
    // main window is always the first window
    return this->windows.front();
}

LRESULT application::window_proc(HWND handle, UINT message, WPARAM w_param, LPARAM l_param) {
    // find window with this handle
    auto& app = get();
    auto window_it = std::ranges::find_if(app.windows, [handle](const std::unique_ptr<window>& window) {
        return window->handle == handle;
    });

    if (window_it == app.windows.end()) {
        return DefWindowProc(handle, message, w_param, l_param);
    }

    return (*window_it)->window_proc(message, w_param, l_param) ? 0 : DefWindowProc(handle, message, w_param, l_param);
}
