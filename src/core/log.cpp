#include "log.hpp"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/dup_filter_sink.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/stdout_sinks.h>

void log::init(const std::filesystem::path& folder) {
    auto dup_filter = std::make_shared<spdlog::sinks::dup_filter_sink_st>(std::chrono::seconds(2));
    
    const auto msvc_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
    const auto stdout_sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();

    const std::filesystem::path log_file = folder / "Current.log";
    const std::filesystem::path previous_logs_folder = folder / "Previous";

    // creates all required directories at once
    std::filesystem::create_directories(previous_logs_folder);

    if (std::filesystem::exists(log_file)) {
        const auto creation_time = std::filesystem::last_write_time(log_file).time_since_epoch();
        const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(creation_time);
        std::string time_str = std::to_string(seconds.count());

        std::filesystem::path previous_log_file = previous_logs_folder / (time_str + ".log");
        MoveFile(log_file.c_str(), previous_log_file.c_str());
    }
    
    const auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file.string());

    dup_filter->add_sink(msvc_sink);
    dup_filter->add_sink(stdout_sink);
    dup_filter->add_sink(file_sink);

    this->sped_log = std::make_shared<spdlog::logger>("j3", dup_filter);

#ifdef NDEBUG
    sped_log->set_level(spdlog::level::info);
#else // debug
    sped_log->set_level(spdlog::level::debug);
#endif
}
