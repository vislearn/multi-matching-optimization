#ifndef LIBMGM_IO_UTILS_HPP
#define LIBMGM_IO_UTILS_HPP

#include <filesystem>
#include "multigraph.hpp"
#include "solution.hpp"

namespace mgm::io {

std::shared_ptr<MgmModel> parse_dd_file(std::filesystem::path dd_file, double unary_constant=0.0);
std::shared_ptr<GmModel> parse_dd_file_gm(std::filesystem::path dd_file, double unary_constant=0.0);
//MgmModel parse_dd_file_fscan(std::filesystem::path dd_file);

void export_dd_file(std::filesystem::path dd_file, std::shared_ptr<MgmModel> model);

void save_to_disk(std::filesystem::path outPath, const MgmSolution& solution);
void save_to_disk(std::filesystem::path outPath, const GmSolution& solution);
MgmSolution import_from_disk(std::filesystem::path labeling_path, std::shared_ptr<MgmModel> model);

}
#endif