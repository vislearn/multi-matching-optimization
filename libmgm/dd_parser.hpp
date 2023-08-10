#ifndef LIBMGM_DDPARSER_HPP
#define LIBMGM_DDPARSER_HPP

#include <filesystem>
#include "multigraph.hpp"

MgmModel parse_dd_file(std::filesystem::path dd_file);

#endif