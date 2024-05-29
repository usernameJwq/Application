#include "log.h"

#include <string>

#include <spdlog/sinks/rotating_file_sink.h>

void init_logger() {
    std::string log_file = "logs/app.log";
    size_t max_file_size = 1024 * 1024 * 10;
    size_t max_files = 10;
    auto logger = LOG::rotating_logger_mt("App", log_file, max_file_size, max_files);
    LOG::set_default_logger(logger);

    auto log_level = LOG::level::debug;
    LOG::set_level(log_level);
    LOG::flush_on(log_level);
    LOG::flush_every(std::chrono::milliseconds(1000));
}