#include <memory>
#include <stdexcept>
#include <utility>
#include <algorithm>
#include <iostream>
#include <cassert>
#include <numeric>
#include <cmath>


#include <mpopt/qap.h>
#include "qap_interface.hpp"

#include "multigraph.hpp"

namespace mgm {

constexpr double INFINITY_COST = 1e99;

void QAPSolver::mpopt_Deleter::operator()(mpopt_qap_solver *s) {
    mpopt_qap_solver_destroy(s);
}

QAPSolver::QAPSolver(std::shared_ptr<GmModel> model, int batch_size, int greedy_generations)
    : decomposition(*(model)), model(model), batch_size(batch_size), greedy_generations(greedy_generations)
{    
    // TOGGLE: Supress output from QAP solver
    std::cout.setstate(std::ios_base::failbit);

    auto deleter = QAPSolver::mpopt_Deleter();
    this->mpopt_solver = std::unique_ptr<mpopt_qap_solver, mpopt_Deleter>(mpopt_qap_solver_create(this->estimate_memory_kib()), deleter);
    mpopt_qap_solver_set_fusion_moves_enabled(mpopt_solver.get(), true);
    mpopt_qap_solver_set_local_search_enabled(mpopt_solver.get(), true);
    mpopt_qap_solver_set_dual_updates_enabled(mpopt_solver.get(), true);
    mpopt_qap_solver_use_greedy(mpopt_solver.get());

    if (QAPSolver::libmpopt_seed > 0) {
        mpopt_qap_solver_set_random_seed(mpopt_solver.get(), QAPSolver::libmpopt_seed);
    }

    this->construct_solver();

    // TOGGLE: Supress output from QAP solver   
    std::cout.clear();
}

void QAPSolver::construct_solver() {
    auto g = mpopt_qap_solver_get_graph(this->mpopt_solver.get());
    auto m = this->model;
    auto& deco = this->decomposition;

    // Insert unary factors
    for (int qap_node = 0; qap_node < deco.no_qap_nodes; qap_node++) {
        int gm_node = deco.gm_id(qap_node);
        auto& gm_node_assignments = m->assignments_left[gm_node];

        int no_b = this->decomposition.no_backward[gm_node];
        int no_f = this->decomposition.no_forward[gm_node];

        int no_connections = gm_node_assignments.size() + 1;
        auto unary = mpopt_qap_graph_add_unary(g, qap_node, no_connections, no_f, no_b);

        for (size_t idx = 0; idx < gm_node_assignments.size(); idx++) {
            int gm_label = gm_node_assignments[idx];
            mpopt_qap_unary_set_cost(unary, idx, m->costs->unary(gm_node, gm_label));
        }
        mpopt_qap_unary_set_cost(unary, gm_node_assignments.size(), 0.0); // dummy node (label for leaving node unassigned)
    }

    // Insert uniqueness factors
    for (size_t gm_label = 0; gm_label < m->assignments_right.size(); gm_label++) {
        auto& gm_label_assignments = m->assignments_right[gm_label];

        int no_assignments = gm_label_assignments.size();

        (void) mpopt_qap_graph_add_uniqueness(g, gm_label, no_assignments, gm_label);

        for (size_t idx = 0; idx < gm_label_assignments.size(); idx++) {
            int gm_node = gm_label_assignments[idx];

            // This finds the position of gm_label in the assignment list associated with gm_node.
            // E.g. gm_label in assignments_right has 20 associated assignments, where gm_node is at position 5
            // Assume also 20 associated assignments for gm_node in assignments_left.
            //
            // Here we need to find for the given gm_node, which position the original gm_label has, to link the uniqueness constraint correctly.
            auto& gm_node_assignments = m->assignments_left[gm_node];
            auto gm_label_it = std::find(gm_node_assignments.begin(), gm_node_assignments.end(), gm_label); // FIXME: O(n) is best avoided.
            int gm_label_idx = std::distance(gm_node_assignments.begin(), gm_label_it);

            // FIXME: DEBUG AND TEST IF THIS IS CORRECT
            int qap_node = deco.qap_id(gm_node);
            mpopt_qap_graph_add_uniqueness_link(g, qap_node, gm_label_idx, gm_label, idx);
        }
    }

    // Insert pairwise factors
    int pairwise_idx = 0;
    for (auto& [gm_node1, node1_pairwise]: this->decomposition.pairwise) {
        for (auto& [gm_node2, costs]: node1_pairwise) {
            auto p = mpopt_qap_graph_add_pairwise(g, pairwise_idx, costs.size(), costs[0].size());
            int qap_node1 = deco.qap_id(gm_node1);
            int qap_node2 = deco.qap_id(gm_node2);
            mpopt_qap_graph_add_pairwise_link(g, qap_node1, qap_node2, pairwise_idx);
            
            assert(costs.size() == m->assignments_left[gm_node1].size() + 1);
            assert(costs[0].size() == m->assignments_left[gm_node2].size() + 1);

            for (size_t c_i = 0; c_i < costs.size(); c_i++) {
                for (size_t c_j = 0; c_j < costs[0].size(); c_j++) {
                    mpopt_qap_pairwise_set_cost(p, c_i, c_j, costs[c_i][c_j]);
                }
            }
            pairwise_idx++;
        }
    }
}

GmSolution QAPSolver::run(bool verbose) {
    // TOGGLE: Supress output from QAP solver
    if (!verbose) 
        std::cout.setstate(std::ios_base::failbit);

    mpopt_qap_solver_set_stopping_criterion(this->mpopt_solver.get(), this->stopping_criteria.p,this->stopping_criteria.k);
    mpopt_qap_solver_run(this->mpopt_solver.get(), this->batch_size, this->stopping_criteria.max_batches, this->greedy_generations);

    // UNTOGGLE: Supress output from QAP solver
    if (!verbose) 
        std::cout.clear();

    return this->extract_solution();
}

GmSolution QAPSolver::extract_solution() {
    GmSolution solution(this->model);
    auto g = mpopt_qap_solver_get_graph(this->mpopt_solver.get());

    for (int qap_node = 0; qap_node < this->decomposition.no_qap_nodes; qap_node++) {
        unsigned long lib_primal = mpopt_qap_unary_get_primal(mpopt_qap_graph_get_unary(g, qap_node));
        int gm_node = this->decomposition.gm_id(qap_node);

        if (lib_primal < this->model->assignments_left[gm_node].size()) {
            int label = this->model->assignments_left[gm_node][lib_primal];
            solution[gm_node] = label;
        }
        else {
            // Assert dummy node assignment, not out of range.
            assert (lib_primal == this->model->assignments_left[gm_node].size());
        }
    }
    return solution;
}

size_t QAPSolver::estimate_memory_kib()
{
    auto m = this->model;
    auto& deco = this->decomposition;

    size_t  unary_inserts       = 0;
    size_t  uniqueness_inserts  = 0;
    size_t  pairwise_inserts    = 0;

    // Count for unary factors
    for (const auto& v : m->assignments_left) {
        unary_inserts += v.size();
    }
    unary_inserts *= 2; // no_connections for unaries is added twice in libmpopt.

    unary_inserts += std::reduce(deco.no_backward.begin(),  deco.no_backward.end());
    unary_inserts += std::reduce(deco.no_forward.begin(),   deco.no_forward.end());

    // Count for uniqueness factors
    for (const auto& v : m->assignments_right) {
        uniqueness_inserts += v.size();
    }
    uniqueness_inserts *= 2;
    uniqueness_inserts += m->assignments_right.size(); // +1 on every factor for dummy nodes.

    // Count pairwise factors
    for (auto& [gm_node1, node1_pairwise]: deco.pairwise) {
        for (auto& [gm_node2, costs]: node1_pairwise) {
            //auto p = mpopt_qap_graph_add_pairwise(g, pairwise_idx, costs.size(), costs[0].size());
            pairwise_inserts += (costs.size() * costs[0].size());
        }
    }
    size_t estimate = 0;
    size_t total_inserts = unary_inserts + uniqueness_inserts + pairwise_inserts;

    if (total_inserts > 1024) {
        estimate = 2 + ( std::ceil(total_inserts / 1024) * sizeof(double) );
    }
    else {
        estimate = 2 + ( std::ceil(total_inserts * sizeof(double)) / 1024 );
    } 

    // The estimate buffers with a factor 3.5 due to 
    // potential alignment optimizations in mpopt::block_allocator (void align(size_t a) in allocator.hpp)
    // and overhead of mpopt datastructures used to store values in libmpopt. (fixed_vector.hpp)
    return estimate * 3.5;
}

namespace details
{

ModelDecomposition::ModelDecomposition(const GmModel& model) {
    
    // FIXME: This could be skipped unless necessary.
    // Remap indices. Unneccessary in most cases, but there sadly are some edge cases.
    this->qap_node_id_to_model_node_id.reserve(model.graph1.no_nodes);
    this->model_node_id_to_qap_node_id.reserve(model.graph1.no_nodes);
    for (size_t i = 0; i < model.assignments_left.size(); i++) {
        if (!model.assignments_left[i].empty()) {
            model_node_id_to_qap_node_id[i] = qap_node_id_to_model_node_id.size();
            qap_node_id_to_model_node_id.push_back(i);
        }
    }
    this->no_qap_nodes  = qap_node_id_to_model_node_id.size();

    this->no_backward   = std::vector<int>(model.graph1.no_nodes, 0);
    this->no_forward    = std::vector<int>(model.graph1.no_nodes, 0);

    // add pairwise edges
    this->pairwise.reserve(model.graph1.no_nodes);
    for (const auto& edge : model.costs->all_edges()) {
        insert_pairwise(model, edge.first, edge.second);
    }

    // Set pairwise edge cost to infinity for prohibiting assignment constraints.
    for (size_t label = 0; label < model.assignments_right.size(); label++) {
        auto label_ass = model.assignments_right[label];
        if (label_ass.size() < 2) {
            continue;
        }
        for (auto it = label_ass.begin(); it != label_ass.end() - 1; it++) {
            for (auto it2 = it+1; it2 != label_ass.end(); it2++) {
                EdgeIdx e(AssignmentIdx(*it, label), AssignmentIdx(*it2, label));
                insert_pairwise(model, e, INFINITY_COST, false);
            }
        } 
    }
}

// FIXME: Rename identifiers to avoid any confusion between Multigraph and QAP graph models.
void ModelDecomposition::insert_pairwise(const GmModel& model, const EdgeIdx& edge, const double& cost, bool create_new_edges) {
    AssignmentIdx a1;
    AssignmentIdx a2;
    if (edge.first.first < edge.second.first) {
        a1 = edge.first;
        a2 = edge.second;
    } 
    else {
        a1 = edge.second;
        a2 = edge.first;
    }

    // Check if pairwise coss between a1.first and a2.first exists
    auto& a1_pairwise = this->pairwise[a1.first];
    auto pairwise_costs = a1_pairwise.find(a2.first);
    if (create_new_edges && (pairwise_costs == a1_pairwise.end())) {
        assert(a1.first < a2.first);
        // no edge
        // create new pairwise cost structure
        this->no_forward[a1.first] += 1;
        this->no_backward[a2.first] += 1;
        
        // cost structure. +1 for dummy nodes.
        std::pair<int, int> shape;
        shape.first = model.assignments_left[a1.first].size() + 1;
        shape.second = model.assignments_left[a2.first].size() + 1;

        // Cost matrix between two assignments
        auto cost_structure = DecompCosts(shape.first, std::vector<double>(shape.second, 0.0));

        pairwise_costs = a1_pairwise.insert(std::make_pair(a2.first, std::move(cost_structure))).first;
    } 
    else if (!create_new_edges) {
        // in case the edges did not exist, and nothing should be done with them.
        return;
    }

    auto a1_ass = model.assignments_left[a1.first];
    int pos1 = std::distance(a1_ass.begin(), std::find(a1_ass.begin(), a1_ass.end(), a1.second));

    auto a2_ass = model.assignments_left[a2.first];
    int pos2 = std::distance(a2_ass.begin(), std::find(a2_ass.begin(), a2_ass.end(), a2.second));
    
    assert(pairwise_costs->second[pos1][pos2] == 0.0 || pairwise_costs->second[pos1][pos2] == cost);
    pairwise_costs->second[pos1][pos2] = cost;
}

int ModelDecomposition::gm_id(int qap_node_id) {
    return this->qap_node_id_to_model_node_id[qap_node_id];
}

int ModelDecomposition::qap_id(int gm_node_id) {
    return this->model_node_id_to_qap_node_id[gm_node_id];
}
}
}
