#ifndef LIBMGM_MULTIGRAPH_HPP
#define LIBMGM_MULTIGRAPH_HPP

#include <unordered_map>
#include <vector>
#include <format>
#include <string>
#include <memory>
#include <utility>

#include "datastructures.hpp"


typedef std::pair<int,int> GmModelIdx;

struct GmModelIdxHash {
    std::size_t operator()(GmModelIdx const& input) const noexcept {
        std::string s = std::to_string(input.first) + ',' + std::to_string(input.second);
        std::size_t hash = std::hash<std::string>{}(s);

        return hash;
    }
};

class Graph {
    public:
        Graph() {};
        Graph(int id, int no_nodes);

        int id;
        int no_nodes;
};

class GmModel {
    public:
        GmModel() {};
        GmModel(Graph g1, Graph g2, int no_assignments, int no_edges);
        Graph graph1;
        Graph graph2;

        int no_assignments;
        int no_edges;

        std::vector<AssignmentIdx> assignment_list;
        std::vector<EdgeIdx> edge_list;
        std::vector<std::vector<int>> assignments_left;
        std::vector<std::vector<int>> assignments_right;
        std::unique_ptr<ICostStructure> costs;
};

class MgmModel {
    public:
        MgmModel();
        std::unordered_map<GmModelIdx, std::shared_ptr<GmModel>, GmModelIdxHash> models;
};

#endif