#include "application.hpp"

#include "framework/service/internet/internet.hpp"
#include "special_folder.hpp"

alignas(application) char application_buffer[sizeof(application)];

application::application(const HINSTANCE instance) {
    const std::filesystem::path log_folder = special_folder::get(FOLDERID_LocalAppData) / "J3" / "Logs";
    this->log.init(log_folder);

    this->instance = instance;
    this->log.info("Application start");

    winrt::init_apartment();
    this->log.debug("Initialized WinRT apartment");

    // get icon because i don't want to load one from a file
    std::array<wchar_t, MAX_PATH> module_path = {};
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

    this->log.debug("Window class registered for application");
    this->log.debug("Application singleton initialized");
}

application& application::get() { return *std::launder(reinterpret_cast<application*>(application_buffer)); }

void application::run() {
    if (this->windows.empty()) {
        // handle error
    }

    this->running = true;

    const auto& main_window = get_main_window();
    main_window->show();

    // window loop
    this->log.info("Now running");
    MSG message = {};
    while (running) {
        bool has_message = get_message(
            message,
            main_window->get_focused()
            ? static_cast<UINT>(Rml::Math::Min(Rml::GetContext("main")->GetNextUpdateDelay(), 10.0) * 1000) : 100
        );

        while (has_message) {
            TranslateMessage(&message);
            DispatchMessage(&message);

            if (message.message == WM_QUIT) {
                this->running = false;
                break;
            }

            has_message = get_message(message, 0);
        }

        // perform updates during dead time
        this->time.update();

        for (const auto& window : this->windows) {
            window->update();
        }
    }

    this->log.info("Application end");
}

void application::quit(const int exit_code) {
    if (!this->running) {
        this->log.warn("Exiting immediately");
        exit(exit_code);
    }

    this->log.debug("Application exit requested");
    PostQuitMessage(exit_code);
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

bool application::get_message(MSG& message, UINT timeout) {
    if (timeout != 0) {
        UINT_PTR timer_id = SetTimer(nullptr, 0, timeout, nullptr);
        BOOL result = GetMessage(&message, nullptr, 0, 0);
        KillTimer(nullptr, timer_id);

        if (message.message != WM_TIMER || message.hwnd != nullptr || message.wParam != timer_id) return result;
    }

    return PeekMessage(&message, nullptr, 0, 0, PM_REMOVE);
}
