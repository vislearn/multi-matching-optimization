#include "multigraph.hpp"

#include <algorithm>
#include <utility>
#include <stdexcept>
#include <sstream>
#include <cmath>

namespace mgm {
    
Graph::Graph(int id, int no_nodes) : id(id), no_nodes(no_nodes) {
    if (no_nodes < 0) {
        std::ostringstream msg;
        msg << "Number of nodes must be positive, got " << no_nodes;
        throw std::invalid_argument(msg.str());
    }
};

 //TODO: Think about removing this. Code will throw an exception if more assignments/edges are added then buffered here.
GmModel::GmModel(Graph g1, Graph g2)
    : 
GmModel::GmModel(g1, g2, 100, 1000) {}

GmModel::GmModel(Graph g1, Graph g2, int no_assignments, int no_edges) 
    : 
    graph1(g1), 
    graph2(g2)
    {
    // Validate graphs are valid
    if (g1.id < 0 || g2.id < 0) {
        throw std::invalid_argument("Graph IDs must be non-negative");
    }
    if (g1.id == g2.id) {
        std::ostringstream msg;
        msg << "Cannot create GmModel with identical graph IDs (" << g1.id << ")";
        throw std::invalid_argument(msg.str());
    }
    
    // Validate input parameters
    if (no_assignments < 0) {
        std::ostringstream msg;
        msg << "Number of assignments must be non-negative, got " << no_assignments;
        throw std::invalid_argument(msg.str());
    }
    if (no_assignments > g1.no_nodes * g2.no_nodes) {
        std::ostringstream msg;
        msg << "Number of assignments larger than product of graph node counts. Got " << no_assignments << " Max: " << g1.no_nodes * g2.no_nodes;
        throw std::invalid_argument(msg.str());
    }
    if (no_edges < 0) {
        std::ostringstream msg;
        msg << "Number of edges must be non-negative, got " << no_edges;
        throw std::invalid_argument(msg.str());
    }
    
    this->costs = std::make_unique<CostMap>(no_assignments, no_edges);
    this->assignment_list.reserve(no_assignments);

    //FIXME: Number of elements for assignments_left and assignments_right is unclear.
    // Loading assignments without reserving space leads to (avoidable?) reallocations. 
    this->assignments_left  = std::vector<std::vector<int>>(g1.no_nodes);
    this->assignments_right = std::vector<std::vector<int>>(g2.no_nodes);
}

int GmModel::no_assignments() const
{
    return this->costs->all_assignments().size();
}

int GmModel::no_edges() const
{
    return this->costs->all_edges().size();
}

void GmModel::add_assignment(int node1, int node2, double cost)
{
    // Validate node indices are within bounds
    if (node1 < 0 || node1 >= graph1.no_nodes) {
        std::ostringstream msg;
        msg << "Node1 index " << node1 << " is out of range [0, " << graph1.no_nodes << ")";
        throw std::out_of_range(msg.str());
    }
    if (node2 < 0 || node2 >= graph2.no_nodes) {
        std::ostringstream msg;
        msg << "Node2 index " << node2 << " is out of range [0, " << graph2.no_nodes << ")";
        throw std::out_of_range(msg.str());
    }
    
    // Validate cost is not NaN (only NaN is not equal to itself)
    if (std::isnan(cost)) {
        throw std::invalid_argument("Cost cannot be NaN");
    }
    
    // Check if assignment already exists
    if (this->costs->contains(node1, node2)) {
        std::ostringstream msg;
        msg << "Assignment (" << node1 << ", " << node2 << ") already exists";
        throw std::runtime_error(msg.str());
    }
    
    (void) this->assignment_list.emplace_back(node1, node2);

    this->costs->set_unary(node1, node2, cost);
    this->assignments_left[node1].push_back(node2);
    this->assignments_right[node2].push_back(node1);
}

void GmModel::add_edge(int assignment1, int assignment2, double cost) {
    // Validate assignment indices are within bounds
    if (assignment1 < 0 || assignment1 >= static_cast<int>(this->assignment_list.size())) {
        std::ostringstream msg;
        msg << "Assignment1 index " << assignment1 << " is out of range [0, " << this->assignment_list.size() << ")";
        throw std::out_of_range(msg.str());
    }
    if (assignment2 < 0 || assignment2 >= static_cast<int>(this->assignment_list.size())) {
        std::ostringstream msg;
        msg << "Assignment2 index " << assignment2 << " is out of range [0, " << this->assignment_list.size() << ")";
        throw std::out_of_range(msg.str());
    }
    
    auto& a1 = this->assignment_list[assignment1];
    auto& a2 = this->assignment_list[assignment2];

    this->add_edge(a1.first, a1.second, a2.first, a2.second, cost);
}

void GmModel::add_edge(int assignment1_node1, int assignment1_node2, int assignment2_node1, int assignment2_node2, double cost) {
    // Validate cost is not NaN (only NaN is not equal to itself)
    if (std::isnan(cost)) {
        throw std::invalid_argument("Cost cannot be NaN");
    }
    
    // Prevent self-edges
    if (assignment1_node1 == assignment2_node1 || assignment1_node2 == assignment2_node2) {
        std::ostringstream msg;
        msg << "Cannot create edge. "
            << "Pairwise costs require 4 unique nodes."
            << "Given: (" << assignment1_node1 << ", " << assignment1_node2 << ")"
                << " - (" << assignment2_node1 << ", " << assignment2_node2 << ")";
        throw std::invalid_argument(msg.str());
    }
    
    // Validate that both assignments exist
    if (!this->costs->contains(assignment1_node1, assignment1_node2)) {
        std::ostringstream msg;
        msg << "Assignment (" << assignment1_node1 << ", " << assignment1_node2 << ") does not exist. Add it before creating edges.";
        throw std::runtime_error(msg.str());
    }
    if (!this->costs->contains(assignment2_node1, assignment2_node2)) {
        std::ostringstream msg;
        msg << "Assignment (" << assignment2_node1 << ", " << assignment2_node2 << ") does not exist. Add it before creating edges.";
        throw std::runtime_error(msg.str());
    }
    
    // Check if edge already exists
    if (this->costs->contains(assignment1_node1, assignment1_node2, assignment2_node1, assignment2_node2)) {
        std::ostringstream msg;
        msg << "Edge between assignments (" << assignment1_node1 << ", " << assignment1_node2 
            << ") and (" << assignment2_node1 << ", " << assignment2_node2 << ") already exists";
        throw std::runtime_error(msg.str());
    }
    
    this->costs->set_pairwise(assignment1_node1, assignment1_node2, assignment2_node1, assignment2_node2, cost);
    //this->costs->set_pairwise(a2.first, a2.second, a1.first, a1.second, cost); //FIXME: RAM overhead. Avoids sorting later though.
}


MgmModel::MgmModel(){ 
    //models.reserve(300);
}

std::shared_ptr<MgmModel> MgmModel::create_submodel(std::vector<int> graph_ids)
{
    // Validate input
    if (graph_ids.empty()) {
        throw std::invalid_argument("Cannot create submodel with empty graph_ids");
    }
    
    auto submodel = std::make_shared<MgmModel>();
    submodel->no_graphs = graph_ids.size();
    submodel->graphs.reserve(submodel->no_graphs);

    std::sort(graph_ids.begin(), graph_ids.end());
    
    // Check for duplicates after sorting
    auto it = std::adjacent_find(graph_ids.begin(), graph_ids.end());
    if (it != graph_ids.end()) {
        std::ostringstream msg;
        msg << "Duplicate graph ID " << *it << " in graph_ids";
        throw std::invalid_argument(msg.str());
    }
    
    for (const auto & id : graph_ids) {
        if (id < 0 || id >= this->no_graphs) {
            std::ostringstream msg;
            msg << "Can't create submodel. Graph ID " << id << " is out of range [0, " << this->no_graphs << ")";
            throw std::out_of_range(msg.str());
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