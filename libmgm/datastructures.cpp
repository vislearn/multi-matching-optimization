#include <memory>
#include <utility>
#include "datastructures.hpp"

// Has to be defined, as ICostStructure has no other method that could be declared as pure virtual.
ICostStructure::~ICostStructure() {}

const double& ICostStructure::unary(AssignmentIdx assignment) {
    return this->unary(assignment.first, assignment.second);
}

const double& ICostStructure::pairwise(EdgeIdx edge) {
    return this->pairwise(edge.first.first, edge.first.second, edge.second.first, edge.second.second);
}

CostMap::CostMap(int no_nodes_g1, int no_unaries, int no_pairwise) {
    //FIXME: Number of elements for assignments_left and assignments_right is unclear.
    // Loading assignments without reserving space leads to (avoidable?) reallocations. 
    this->assignments.reserve(no_nodes_g1);
    this->edges.reserve(no_pairwise);
}

const double& CostMap::unary(int node1, int node2) {
    return this->assignments.at(node1).at(node2);
}

const double& CostMap::pairwise(int node1, int node2, int node3, int node4) {
    AssignmentIdx a1 = AssignmentIdx(node1, node2);
    AssignmentIdx a2 = AssignmentIdx(node3, node4);

    return this->edges[EdgeIdx(a1, a2)];
}

void CostMap::set_unary(int node1, int node2, double cost) {
    this->assignments[node1][node2] = cost;
}

void CostMap::set_pairwise(int node1, int node2, int node3, int node4, double cost) {
    AssignmentIdx a1 = AssignmentIdx(node1, node2);
    AssignmentIdx a2 = AssignmentIdx(node3, node4);

    this->edges[EdgeIdx(a1, a2)] = cost;
}