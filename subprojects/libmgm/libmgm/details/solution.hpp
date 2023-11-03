#ifndef LIBMGM_SOLUTION_HPP
#define LIBMGM_SOLUTION_HPP

#include <unordered_map>
#include <vector>
#include <memory>
#include <utility>

#include "cliques.hpp"
#include "multigraph.hpp"

namespace mgm {

class GmSolution {
    public:
        GmSolution() = default;
        GmSolution(std::shared_ptr<GmModel> model);
        std::vector<int> labeling;

        double evaluate() const;
        std::shared_ptr<GmModel> model;

    private:
        bool is_active(AssignmentIdx assignment) const;
};

class MgmSolution {
    public:
        MgmSolution(std::shared_ptr<MgmModel> model);

        std::unordered_map<GmModelIdx, GmSolution, GmModelIdxHash> gmSolutions;
        std::shared_ptr<MgmModel> model;

        void build_from(const CliqueTable& cliques);
        CliqueTable export_cliquetable();

        double evaluate() const;
        bool is_cycle_consistent() const;
};

}
#endif