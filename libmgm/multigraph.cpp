#include "multigraph.hpp"

#include <utility>

Graph::Graph(int id, int no_nodes) : id(id), no_nodes(no_nodes) {};

GmModel::GmModel(Graph g1, Graph g2, int no_assignments, int no_edges) : graph1(g1), graph2(g2) {
    this->costs = std::make_unique<CostMap>(g1.no_nodes, no_assignments, no_edges);
    this->assignment_list.reserve(no_assignments);
    this->assignments_left  = std::vector<std::vector<int>>(g1.no_nodes);
    this->assignments_right = std::vector<std::vector<int>>(g2.no_nodes);
};

MgmModel::MgmModel(){ 
    //models.reserve(300);
};