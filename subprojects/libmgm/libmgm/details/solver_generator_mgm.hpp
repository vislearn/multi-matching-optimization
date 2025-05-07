#ifndef LIBMGM_SOLVER_GENERATOR_MGM_HPP
#define LIBMGM_SOLVER_GENERATOR_MGM_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <queue>

#include "cliques.hpp"
#include "multigraph.hpp"
#include "solution.hpp"

namespace mgm {

class MgmGenerator {
    public:
        enum matching_order {
            sequential,
            random
        };

    protected:
        MgmGenerator(std::shared_ptr<MgmModel> model);
        virtual ~MgmGenerator() = default;

        virtual MgmSolution generate() = 0;

        std::vector<int> init_generation_sequence(matching_order order);
        std::vector<int> generation_sequence; //Remember the order in which graphs were added

        MgmSolution current_state;
        std::shared_ptr<MgmModel> model;
};

class SequentialGenerator : public MgmGenerator {
    public:
        SequentialGenerator(std::shared_ptr<MgmModel> model);

        MgmSolution generate() override;
        void step();

        std::vector<int> init(matching_order order);

    protected:
        std::queue<CliqueManager> generation_queue;

        int current_step = 0;
};

class ParallelGenerator : public MgmGenerator {
    public:
        ParallelGenerator(std::shared_ptr<MgmModel> model);
        MgmSolution generate() override;

        std::vector<int> init(matching_order order);

    private:
        std::vector<CliqueManager> generation_queue;
        CliqueManager parallel_task(std::vector<CliqueManager> sub_generation);
};

namespace details {
//FIXME: Try to remove this MgmModel& dependency.
// Maybe not ideal to have these functions outside any class.
// Needed for MgmSolver and Local searcher (-> Parent class maybe?)
GmSolution match(const CliqueManager& manager_1, const CliqueManager& manager_2, const MgmModel& model);
CliqueManager merge(const CliqueManager& manager_1, const CliqueManager& manager_2, const GmSolution& solution, const MgmModel& model);
std::pair<CliqueManager, CliqueManager> split(const CliqueManager& manager, int graph_id, const MgmModel& model); // Splits off graph [graph_id] from manager
        

class CliqueMatcher {
    public:
        CliqueMatcher(const CliqueManager& manager_1, const CliqueManager& manager_2, const MgmModel& model);
        GmSolution match();

    private:
        const CliqueManager& manager_1;
        const CliqueManager& manager_2;
        const MgmModel& model;

        GmModel construct_qap();
        void collect_assignments();
        void collect_edges();

        GmModel construct_gm_model();

        // Maps assignment_indices from original GmModels
        // to asignment_indices in clique-to-clique matching model
        // [graph1_id, graph2_id] -> [assignment_idx] -> [clique_pair_idx]
        using CliqueAssignmentIdx = AssignmentIdx;
        //std::unordered_map<GmModelIdx, std::unordered_map<AssignmentIdx, CliqueAssignmentIdx, AssignmentIdxHash>, GmModelIdxHash> assignment_idx_map;

        // AssignmentIdx is a pair of clique_ids here, as Cliques are matched to each other.
        std::unordered_map<CliqueAssignmentIdx, std::vector<double>, AssignmentIdxHash> clique_assignments;
        std::unordered_map<EdgeIdx, double, EdgeIdxHash> clique_edges;
};
}

}

#endif