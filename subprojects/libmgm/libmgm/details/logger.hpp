#ifndef LIBMGM_LOGGER_HPP
#define LIBMGM_LOGGER_HPP

#include <filesystem>
#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

namespace mgm {
    
void init_logger(fs::path outPath, std::string filename, spdlog::level::level_enum loglevel);

}
#endif