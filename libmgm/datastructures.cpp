#include "datastructures.hpp"
#include <memory>
#include <utility>

// Has to be defined, as ICostStructure has no other method that could be declared as pure virtual.
ICostStructure::~ICostStructure() {}

CostMap::CostMap(int no_unaries, int no_pairwise) {
    this->assignments.reserve(no_unaries);
    this->edges.reserve(no_pairwise);
}

double& CostMap::unary(int node1, int node2) {
    return this->assignments[AssignmentIdx(node1, node2)];
}

double& CostMap::pairwise(int node1, int node2, int node3, int node4) {
    AssignmentIdx a1 = AssignmentIdx(node1, node2);
    AssignmentIdx a2 = AssignmentIdx(node3, node4);

    return this->edges[EdgeIdx(a1, a2)];
}