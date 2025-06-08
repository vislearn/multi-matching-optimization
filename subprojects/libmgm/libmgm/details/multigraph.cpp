#include "multigraph.hpp"

#include <algorithm>
#include <utility>

namespace mgm {
    
Graph::Graph(int id, int no_nodes) : id(id), no_nodes(no_nodes) {};

 //TODO: Think about removing this. Code will throw an exception if more assignments/edges are added then buffered here.
GmModel::GmModel(Graph g1, Graph g2)
    : 
GmModel::GmModel(g1, g2, 100, 1000) {}

GmModel::GmModel(Graph g1, Graph g2, int no_assignments, int no_edges) 
    : 
    graph1(g1), 
    graph2(g2)
    {
    this->costs = std::make_unique<CostMap>(no_assignments, no_edges);
    this->assignment_list.reserve(no_assignments);

    //FIXME: Number of elements for assignments_left and assignments_right is unclear.
    // Loading assignments without reserving space leads to (avoidable?) reallocations. 
    this->assignments_left  = std::vector<std::vector<int>>(g1.no_nodes);
    this->assignments_right = std::vector<std::vector<int>>(g2.no_nodes);
}

int GmModel::no_assignments()
{
    return this->costs->all_assignments().size();
}

int GmModel::no_edges()
{
    return this->costs->all_edges().size();
}

void GmModel::add_assignment(int node1, int node2, double cost)
{
    // TODO: It should be checked, that node1/node2 is in range of nodes of graphs.
    (void) this->assignment_list.emplace_back(node1, node2);

    this->costs->set_unary(node1, node2, cost);
    this->assignments_left[node1].push_back(node2);
    this->assignments_right[node2].push_back(node1);
}

void GmModel::add_edge(int assignment1, int assignment2, double cost) {
    auto& a1 = this->assignment_list[assignment1];
    auto& a2 = this->assignment_list[assignment2];

    this->add_edge(a1.first, a1.second, a2.first, a2.second, cost);
}

void GmModel::add_edge(int assignment1_node1, int assignment1_node2, int assignment2_node1, int assignment2_node2, double cost) {
    this->costs->set_pairwise(assignment1_node1, assignment1_node2, assignment2_node1, assignment2_node2, cost);
    //this->costs->set_pairwise(a2.first, a2.second, a1.first, a1.second, cost); //FIXME: RAM overhead. Avoids sorting later though.
}


MgmModel::MgmModel(){ 
    //models.reserve(300);
}

std::shared_ptr<MgmModel> MgmModel::create_submodel(std::vector<int> graph_ids)
{
    auto submodel = std::make_shared<MgmModel>();
    submodel->no_graphs = graph_ids.size();
    submodel->graphs.reserve(submodel->no_graphs);

    std::sort(graph_ids.begin(), graph_ids.end());
    for (const auto & id : graph_ids) {
        if (id < 0 || id >= this->no_graphs) {
            throw std::out_of_range("Can't create submodel. Graph ID out of range");
        }
        submodel->graphs.push_back(this->graphs[id]);
    }
    for (const auto & [key, gm_model] : this->models) {
        if (std::find(graph_ids.begin(), graph_ids.end(), key.first) != graph_ids.end() &&
            std::find(graph_ids.begin(), graph_ids.end(), key.second) != graph_ids.end()) {
            submodel->models[key] = gm_model;
        }
    }

    return submodel;
}
}