#ifndef LIBMGM_IO_UTILS_HPP
#define LIBMGM_IO_UTILS_HPP

#include <filesystem>
#include "multigraph.hpp"
#include "solution.hpp"

namespace mgm::io {

MgmModel parse_dd_file(std::filesystem::path dd_file, double unary_constant=0.0);
MgmModel parse_dd_file_fscan(std::filesystem::path dd_file);

void export_dd_file(std::filesystem::path dd_file, std::shared_ptr<MgmModel> model);

void safe_to_disk(const MgmSolution& solution, std::filesystem::path outPath, std::string filename);
MgmSolution import_from_disk(std::shared_ptr<MgmModel> model, std::filesystem::path labeling_path);

}
#endif