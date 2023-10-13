#include <functional>
#include <unordered_map>
#include <string>
#include <algorithm>

#include "costs.hpp"

namespace mgm {
    
CostMap::CostMap(int no_unaries, int no_pairwise) {
    this->assignments.reserve(no_unaries);
    this->edges.reserve(no_pairwise);
}

const double& CostMap::unary(int node1, int node2) const{
    return this->unary(AssignmentIdx(node1, node2));
}

const double& CostMap::unary(AssignmentIdx assignment) const{
    return this->assignments.at(assignment);
}

const double& CostMap::pairwise(int node1, int node2, int node3, int node4) const {
    AssignmentIdx a1 = AssignmentIdx(node1, node2);
    AssignmentIdx a2 = AssignmentIdx(node3, node4);

    return this->pairwise(EdgeIdx(a1, a2));
}

const double& CostMap::pairwise(EdgeIdx edge) const{
    EdgeIdx e = this->sort_edge_indices(edge);
    return this->edges.at(e);
}

bool CostMap::contains (int node1, int node2) const {
    return this->contains(AssignmentIdx(node1, node2));
}

bool CostMap::contains (AssignmentIdx assignment) const {
    return this->assignments.find(assignment) != this->assignments.end();
}

bool CostMap::contains (int node1, int node2, int node3, int node4) const {
    AssignmentIdx a1 = AssignmentIdx(node1, node2);
    AssignmentIdx a2 = AssignmentIdx(node3, node4);

    return this->contains(EdgeIdx(a1, a2));
}

bool CostMap::contains (EdgeIdx edge) const {
    EdgeIdx e = this->sort_edge_indices(edge);
    return this->edges.find(e) != this->edges.end();
}

void CostMap::set_unary(int node1, int node2, double cost) {
    this->assignments[AssignmentIdx(node1, node2)] = cost;
}

void CostMap::set_pairwise(int node1, int node2, int node3, int node4, double cost) {
    AssignmentIdx a1 = AssignmentIdx(node1, node2);
    AssignmentIdx a2 = AssignmentIdx(node3, node4);

    EdgeIdx e = this->sort_edge_indices(EdgeIdx(a1, a2));
    this->edges[e] = cost;
}

// FIXME: Measure impact of this.
EdgeIdx CostMap::sort_edge_indices(EdgeIdx edge) const {
    if (edge.first.first > edge.second.first) {
        return EdgeIdx(edge.second, edge.first);
    }
    return edge;
}

void boost_hash_combine(size_t& seed, const int& v) {
    seed ^= v + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}
}