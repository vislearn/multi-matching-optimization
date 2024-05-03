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

        double evaluate() const;

        int& operator[](int idx); // access labeling
        const int& operator[](int idx) const; // access labeling

        std::vector<int> labeling;
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

        GmSolution& operator[](GmModelIdx idx);
        const GmSolution& operator[](GmModelIdx idx) const;

        double evaluate() const;
        double evaluate(int graph_id) const; // limit cost evaluation to models with graph `graph_id`.
        bool is_cycle_consistent() const;
};

}
#endif