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
        
        std::vector<int> labeling;

        // For use with models saved in memory 
        GmSolution(std::shared_ptr<GmModel> model);
        double evaluate() const;
        std::shared_ptr<GmModel> model;

        // For use with models saved on disc
        GmSolution(std::shared_ptr<GmModel> model, GmModelIdx gmModelIdx);
        double evaluate(const std::shared_ptr<MgmModelBase> mgmModel) const;
        GmModelIdx gmModelIdx;

    private:
        bool is_active(AssignmentIdx assignment) const;
        double evaluate_gm_model(std::shared_ptr<GmModel> gmModel) const;
};

class MgmSolution {
    public:
        MgmSolution(std::shared_ptr<MgmModelBase> model);

        std::unordered_map<GmModelIdx, GmSolution, GmModelIdxHash> gmSolutions;
        std::shared_ptr<MgmModelBase> model;

        void build_from(const CliqueTable& cliques);
        CliqueTable export_cliquetable();

        double evaluate() const;
        bool is_cycle_consistent() const;
};

}
#endif