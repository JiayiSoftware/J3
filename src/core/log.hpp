#pragma once
#include "common.hpp"

class log {
    std::shared_ptr<spdlog::logger> sped_log;
    
public:
    void init(const std::filesystem::path& folder);

    template <typename... args_t>
    void debug(spdlog::format_string_t<args_t...> fmt, args_t&&... args) {
        sped_log->debug(fmt, std::forward<args_t>(args)...);
    }
    
    template <typename... args_t>
    void info(spdlog::format_string_t<args_t...> fmt, args_t&&... args) {
        sped_log->info(fmt, std::forward<args_t>(args)...);
    }
    
    template <typename... args_t>
    void warn(spdlog::format_string_t<args_t...> fmt, args_t&&... args) {
        sped_log->warn(fmt, std::forward<args_t>(args)...);
    }
    
    template <typename... args_t>
    void error(spdlog::format_string_t<args_t...> fmt, args_t&&... args) {
        sped_log->error(fmt, std::forward<args_t>(args)...);
    }
    
    template <typename... args_t>
    void critical(spdlog::format_string_t<args_t...> fmt, args_t&&... args) {
        sped_log->critical(fmt, std::forward<args_t>(args)...);
    }
};
