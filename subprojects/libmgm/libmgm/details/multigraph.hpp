#ifndef LIBMGM_MULTIGRAPH_HPP
#define LIBMGM_MULTIGRAPH_HPP

#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <utility>

#include "costs.hpp"


namespace mgm {

typedef std::pair<int,int> GmModelIdx;

struct GmModelIdxHash {
    std::size_t operator()(GmModelIdx const& input) const noexcept {
        size_t seed = 0;
        boost_hash_combine(seed, input.first);
        boost_hash_combine(seed, input.second);
        return seed;
    }
};

class Graph {
    public:
        Graph() {};
        Graph(int id, int no_nodes);

        int id=-1;
        int no_nodes=-1;
};

class GmModel{
    public:
        GmModel(Graph g1, Graph g2);
        GmModel(Graph g1, Graph g2, int no_assignments, int no_edges);
        Graph graph1;
        Graph graph2;

        int no_assignments();
        int no_edges();

        void add_assignment(int node1, int node2, double cost);

        // both valid alternatives.
        void add_edge(int assignment1, int assigment2, double cost);
        void add_edge(int assignment1_node1, int assignment1_node2, int assignment2_node1, int assignment2_node2, double cost);

        std::vector<AssignmentIdx> assignment_list;
        std::vector<std::vector<int>> assignments_left;
        std::vector<std::vector<int>> assignments_right;
        std::unique_ptr<CostMap> costs;
};

class MgmModel {
    public:
        MgmModel();

        std::shared_ptr<MgmModel> create_submodel(std::vector<int> graph_ids);

        int no_graphs = 0;
        std::vector<Graph> graphs;
        
        std::unordered_map<GmModelIdx, std::shared_ptr<GmModel>, GmModelIdxHash> models;
};

}
#endif