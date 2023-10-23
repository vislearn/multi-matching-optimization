#ifndef LIBMGM_LOGGER_HPP
#define LIBMGM_LOGGER_HPP

#include <filesystem>

namespace fs = std::filesystem;

namespace mgm {
    
void init_logger(fs::path outPath, std::string filename);

}
#endif