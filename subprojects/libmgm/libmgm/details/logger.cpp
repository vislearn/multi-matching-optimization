#include <iostream>
#include <filesystem>

// Logging
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include "logger.hpp"

namespace fs = std::filesystem;
namespace mgm {
    void init_logger(fs::path outPath, std::string filename, spdlog::level::level_enum loglevel) {
        try {
            auto logfile = outPath / fs::path(filename + ".log");

            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logfile.string(), true);

            spdlog::sinks_init_list sink_list = { file_sink, console_sink };

            auto logger = std::make_shared<spdlog::logger>("global_logger", sink_list);
            logger->set_level(loglevel);
            logger->set_pattern("%Y-%m-%d %H:%M:%S,%e  [%^%l%$]\t %v");

            // set for global access
            spdlog::set_default_logger(logger);
        }
        catch (const spdlog::spdlog_ex& ex) {
            std::cout << "Log initialization failed: " << ex.what() << std::endl;
        }
    }
}
