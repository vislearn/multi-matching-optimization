#ifndef LIBMGM_SOLVER_MGM_HPP
#define LIBMGM_SOLVER_MGM_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <queue>

#include "cliques.hpp"
#include "multigraph.hpp"

namespace mgm {

class CliqueManager {
    public:
        // (clique_id, graph_id) -> node_id;
        CliqueManager(Graph g);
        CliqueManager(std::vector<int> graph_ids, const MgmModel& model);
        CliqueTable cliques;
        
        std::vector<int> graph_ids;

        int& clique_idx(int graph_id, int node_id);
        const int& clique_idx(int graph_id, int node_id) const;

        void build_clique_idx_view();
    private:

        // Stores idx of clique in CliqueTable for every node in a graph.
        // [graph_id][node_id] -> clique_idx;
        std::unordered_map<int, std::vector<int>> clique_idx_view;
};

class MgmGenerator {

    public:
        MgmGenerator(std::shared_ptr<MgmModel> model);
        enum generation_order {
            sequential,
            random
        };

        void generate(generation_order order);

        MgmSolution export_solution();
        CliqueTable export_CliqueTable();
        CliqueManager export_CliqueManager();

    private:
        std::unique_ptr<CliqueManager> current_state;
        std::shared_ptr<MgmModel> model;
        std::queue<CliqueManager> generation_queue;

        void init_generation_queue(generation_order order);

        void step();
        GmSolution match(const CliqueManager& manager_1, const CliqueManager& manager_2);
        CliqueManager merge(const CliqueManager& manager_1, const CliqueManager& manager_2, const GmSolution& solution) const;
};

class CliqueMatcher {
    public:
        CliqueMatcher(const CliqueManager& manager_1, const CliqueManager& manager_2, std::shared_ptr<MgmModel> model);
        GmSolution match();

    private:
        const CliqueManager& manager_1;
        const CliqueManager& manager_2;
        std::shared_ptr<MgmModel> model;

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

}

#endif