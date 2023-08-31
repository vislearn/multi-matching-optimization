#ifndef LIBMGM_SOLUTION_HPP
#define LIBMGM_SOLUTION_HPP

#include <map>
#include <vector>
#include <memory>
#include <utility>
#include <filesystem>

#include "multigraph.hpp"

class GmSolution {
    public:
        GmSolution() = default;
        GmSolution(std::shared_ptr<GmModel> model);
        std::vector<int> labeling;

        std::shared_ptr<GmModel> model;
};

class MgmSolution {
    public:
        MgmSolution(std::shared_ptr<MgmModel> model);
        std::unordered_map<GmModelIdx, GmSolution, GmModelIdxHash> gmSolutions;
        std::shared_ptr<MgmModel> model;

        bool is_cycle_consistent();
};

void safe_to_disk(const MgmSolution& solution, std::filesystem::path outPath);

#endif