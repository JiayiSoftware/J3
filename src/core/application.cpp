#include "application.hpp"

#include "special_folder.hpp"

alignas(application) char application_buffer[sizeof(application)];

application::application(const HINSTANCE instance) {
    const std::filesystem::path log_folder = special_folder::get(FOLDERID_LocalAppData) / "J3" / "Logs";
    this->log.init(log_folder);
    
    this->instance = instance;
    this->log.info("Application start");
    
    if (HRESULT hr = CoInitialize(nullptr); FAILED(hr)) { // although this will probably never fail
        this->log.critical("CoInitialize failed with HRESULT 0x{:08X}", hr);
        this->quit(-1);
    }

    this->log.debug("Initialized COM apartment");
    
    // get icon because i don't want to load one from a file
    std::array<wchar_t, MAX_PATH> module_path = { };
    GetModuleFileName(this->instance, module_path.data(), MAX_PATH);
    HICON icon = ExtractIcon(this->instance, module_path.data(), 0);

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

    ATOM atom = RegisterClassEx(&window_class);
    if (atom == 0) {
        this->log.critical("Registering window class failed with result 0x{:08X}", GetLastError());
        this->quit(-1);
        return;
    }

    this->log.debug("Application singleton initialized");
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
    this->log.info("Now running");
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
        time.update();

        for (const auto& window : windows) {
            window->update();
        }
    }

    this->log.info("Application end");
}

void application::quit(const int exit_code) {
    if (!running) {
        this->log.warn("Exiting immediately");
        exit(exit_code);
    }

    this->log.debug("Application exit requested");
    PostQuitMessage(exit_code);
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
