#ifndef LIBMGM_SOLVER_MGM_HPP
#define LIBMGM_SOLVER_MGM_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <queue>

#include "cliques.hpp"
#include "multigraph.hpp"
#include "solution.hpp"

namespace mgm {

class CliqueManager {
    public:
        CliqueManager() = default;
        CliqueManager(Graph g);
        CliqueManager(std::vector<int> graph_ids, const MgmModel& model);

        // (clique_id, graph_id) -> node_id;
        CliqueTable cliques;
        
        std::vector<int> graph_ids;

        int& clique_idx(int graph_id, int node_id);
        const int& clique_idx(int graph_id, int node_id) const;

        void build_clique_idx_view();
        void remove_graph(int graph_id);
    private:

        // Stores idx of clique in CliqueTable for every node in a graph.
        // [graph_id][node_id] -> clique_idx;
        std::unordered_map<int, std::vector<int>> clique_idx_view;
};

class MgmGenerator {

    public:
        MgmGenerator(std::shared_ptr<MgmModel> model);
        enum matching_order {
            sequential,
            random
        };

        void generate(matching_order order);

        std::vector<int> get_generation_sequence();

        MgmSolution export_solution();
        CliqueTable export_CliqueTable();
        CliqueManager export_CliqueManager() const;

    private:
        CliqueManager current_state;
        std::shared_ptr<MgmModel> model;

        std::vector<int> generation_sequence; //Remember the order in which graphs were added
        std::queue<CliqueManager> generation_queue;

        void init_generation_queue(matching_order order);

        void step();
        };

class ParallelGenerator : public MgmGenerator {
    virtual ~ParallelGenerator() = 0;
    class Partitioning {
        Partitioning(int no_graphs);
        void add_partition(std::vector<int> partition);

        private:
            std::vector<std::vector<int>> partitions;
            std::vector<int> graph_id_count;
    };

    public:
        ParallelGenerator(std::shared_ptr<MgmModel> model, Partitioning partitioning);
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