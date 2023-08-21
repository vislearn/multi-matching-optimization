#include <functional>
#include <unordered_map>
#include <string>
#include "datastructures.hpp"


CostMap::CostMap(int no_nodes_g1, int no_unaries, int no_pairwise) {
    this->assignments.reserve(no_nodes_g1);
    this->edges.reserve(no_pairwise);
}

const double& CostMap::unary(int node1, int node2) {
    return this->unary(AssignmentIdx(node1, node2));
}

const double& CostMap::unary(AssignmentIdx assignment) {
    return this->assignments.at(assignment);
}

const double& CostMap::pairwise(int node1, int node2, int node3, int node4) {
    AssignmentIdx a1 = AssignmentIdx(node1, node2);
    AssignmentIdx a2 = AssignmentIdx(node3, node4);

    return this->edges[EdgeIdx(a1, a2)];
}

const double& CostMap::pairwise(EdgeIdx edge) {
    return this->edges.at(edge);
}

void CostMap::set_unary(int node1, int node2, double cost) {
    this->assignments[AssignmentIdx(node1, node2)] = cost;
}

void CostMap::set_pairwise(int node1, int node2, int node3, int node4, double cost) {
    AssignmentIdx a1 = AssignmentIdx(node1, node2);
    AssignmentIdx a2 = AssignmentIdx(node3, node4);

    this->edges[EdgeIdx(a1, a2)] = cost;
}
