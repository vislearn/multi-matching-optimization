#ifndef LIBMGM_LOGGER_HPP
#define LIBMGM_LOGGER_HPP

#include <iostream>
#include <filesystem>

// Logging
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

namespace fs = std::filesystem;

void init_logger(fs::path outPath) {    
    try {
        auto logfile = outPath / fs::path("log/mgm.log");

        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logfile, true);

        spdlog::sinks_init_list sink_list = { file_sink, console_sink };

        auto logger = std::make_shared<spdlog::logger>("global_logger", spdlog::sinks_init_list({console_sink, file_sink}));
        logger->set_level(spdlog::level::debug);
        logger->set_pattern("%Y-%m-%d %H:%M:%S,%e  [%^%l%$]\t %v");

        // set for global access
        spdlog::set_default_logger(logger);
    }
    catch (const spdlog::spdlog_ex& ex) {
        std::cout << "Log initialization failed: " << ex.what() << std::endl;
    }
}

#endif