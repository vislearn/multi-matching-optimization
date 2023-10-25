#include <memory>
#include <vector>
#include <unordered_map>
#include <queue>
#include <exception>
#include <stdexcept>
#include <algorithm>
#include <random>
#include <cassert>
#include <numeric>

// Logging
#include <spdlog/spdlog.h>
#include <fmt/ranges.h> // print vector

#include <omp.h>

#include "cliques.hpp"
#include "multigraph.hpp"
#include "qap_interface.hpp"
#include "random_singleton.hpp"

#include "solver_mgm.hpp"

unsigned int largest_power_of_2_in(unsigned int input);

namespace mgm {

// FIXME: Avoidable loops? Very generic pre-allocation for CliqueManagers of size 1.
CliqueManager::CliqueManager(Graph g) : cliques(1) {
    this->graph_ids.push_back(g.id);

    // Initialize clique table
    this->cliques.reserve(g.no_nodes);
    for (int i = 0; i < g.no_nodes; i++) {
        this->cliques.add_clique();
        this->cliques(i,g.id) = i;
    }

    // Initialize clique view
    this->clique_idx_view[g.id] = std::vector<int>(g.no_nodes);
    std::iota(this->clique_idx_view[g.id].begin(), this->clique_idx_view[g.id].end(), 0);
}

CliqueManager::CliqueManager(std::vector<int> graph_ids, const MgmModel& model) : cliques(graph_ids.size()) {
    this->graph_ids = graph_ids;
    for (auto& id : graph_ids) {
        this->clique_idx_view[id] = std::vector<int>(model.graphs[id].no_nodes, -1);
    }
}

int& CliqueManager::clique_idx(int graph_id, int node_id) {
    return this->clique_idx_view.at(graph_id).at(node_id);
}

const int& CliqueManager::clique_idx(int graph_id, int node_id) const {
    return this->clique_idx_view.at(graph_id).at(node_id);
}

void CliqueManager::build_clique_idx_view() {
    for (auto clique_idx = 0; clique_idx < this->cliques.no_cliques; clique_idx++) {
        for (const auto& c : this->cliques[clique_idx]) {
            this->clique_idx(c.first, c.second) = clique_idx;
        }
    }
}

void CliqueManager::remove_graph(int graph_id, bool should_prune) {
    // assert graph_id is contained in manager
    const auto& idx = std::find(this->graph_ids.begin(), this->graph_ids.end(), graph_id);
    assert(idx != this->graph_ids.end());

    this->graph_ids.erase(idx);
    this->clique_idx_view.erase(graph_id);

    this->cliques.remove_graph(graph_id, false);

    if (should_prune) {
        this->prune();
    }
}

void CliqueManager::prune()
{
    this->cliques.prune();
    this->build_clique_idx_view();
}

void CliqueManager::reconstruct_from(CliqueTable table) {
    this->cliques = table;
    this->build_clique_idx_view();
}

MgmGenerator::MgmGenerator(std::shared_ptr<MgmModel> model) 
    : model(model) {}

MgmSolution MgmGenerator::export_solution() {
    spdlog::info("Exporting solution...");
    MgmSolution sol(this->model);
    sol.build_from(this->current_state.cliques);
    
    return sol;
}

CliqueTable MgmGenerator::export_CliqueTable()
{
    return this->current_state.cliques;
}

CliqueManager MgmGenerator::export_CliqueManager() const
{
    return this->current_state;
}

SequentialGenerator::SequentialGenerator(std::shared_ptr<MgmModel> model) 
    : MgmGenerator(model) {}

void SequentialGenerator::generate() {
    assert (!this->generation_queue.empty());

    // Move first entry in queue to current_state.
    this->current_state = std::move(this->generation_queue.front());
    this->generation_queue.pop();

    int step = 1;
    int no_steps = this->generation_queue.size();

    while (!this->generation_queue.empty()) {
        spdlog::info("Step {}/{}", step, no_steps);
        this->step();
        step++;
    }

    MgmSolution sol(this->model);
    sol.build_from(this->current_state.cliques);

    spdlog::info("Constructed solution. Current energy: {}", sol.evaluate());
    spdlog::info("Finished sequential generation.\n");
}

std::vector<int> SequentialGenerator::init_generation_sequence(matching_order order) {

    // generate sequential order
    std::vector<int> ordering(this->model->no_graphs);
    std::iota(ordering.begin(), ordering.end(), 0);
    
    // shuffle if order should be random
    if (order == random) {
        RandomSingleton::get().shuffle(ordering);
    }

    // Set generation_sequence and generation queue.
    this->generation_sequence = ordering;
    for (const auto& id : ordering) {
        Graph& g = this->model->graphs[id];
        this->generation_queue.emplace(g);
    }

    return ordering;
}

void SequentialGenerator::step() {
    assert(!this->generation_queue.empty());
    CliqueManager& current  = this->current_state;
    CliqueManager& next     = this->generation_queue.front();

    GmSolution solution         = details::match(current, next, (*this->model));
    CliqueManager new_manager   = details::merge(current, next, solution, (*this->model));

    this->current_state = new_manager;
    this->generation_queue.pop();
}

ParallelGenerator::ParallelGenerator(std::shared_ptr<MgmModel> model) 
    : MgmGenerator(model) {}

void ParallelGenerator::generate() {
    
    this->generation_sequence = std::vector<int>(this->model->no_graphs);
    std::iota(this->generation_sequence.begin(), this->generation_sequence.end(), 0);

    spdlog::debug("Parallel Queue: {}", this->generation_sequence);

    std::vector<CliqueManager> queue;
    queue.reserve(this->model->no_graphs);
    
    for (const auto& id : this->generation_sequence) {
        Graph& g = this->model->graphs[id];
        queue.emplace_back(g);
    }


    #pragma omp parallel
    #pragma omp single nowait
    {
        spdlog::debug("Using {} Threads.", omp_get_num_threads());
        this->current_state = parallel_task(queue);
    }
    
    MgmSolution sol(this->model);
    sol.build_from(this->current_state.cliques);

    spdlog::info("Constructed solution. Current energy: {}", sol.evaluate());
    spdlog::info("Finished parallel generation.\n");
}

CliqueManager ParallelGenerator::parallel_task(std::vector<CliqueManager> sub_generation)
{
    unsigned int list_length = sub_generation.size();

    // base case
    if (list_length == 2) {
        spdlog::debug("Merging: {} and {}", sub_generation[0].graph_ids, sub_generation[1].graph_ids);

        GmSolution solution        = details::match(sub_generation[0], sub_generation[1], (*this->model));
        CliqueManager new_manager  = details::merge(sub_generation[0], sub_generation[1], solution, (*this->model));
        return new_manager;
    } else if (list_length == 1) {
        return sub_generation[0];
    } else if (list_length == 0) {
        throw std::length_error("Parallel task receivec empty generation list");
    }
    
    // recursive
    // split into 2 vectors
    auto split_index = largest_power_of_2_in(list_length);

    // list_length is power of 2. Half the list exactly.
    if (split_index == list_length) {
        split_index /= 2;
    }
    std::vector<CliqueManager> half_a(sub_generation.begin(), sub_generation.begin() + split_index);
    std::vector<CliqueManager> half_b(sub_generation.begin() + split_index, sub_generation.end());

    CliqueManager a, b, new_manager;
    
    std::vector<int> ids_a;
    std::vector<int> ids_b;
    for (const auto& g : half_a) {
        ids_a.push_back(g.graph_ids[0]);
    }
    for (const auto& g : half_b) {
        ids_b.push_back(g.graph_ids[0]);
    }
    spdlog::debug("Dispatching Tasks: {} and {}", ids_a, ids_b);
    #pragma omp task untied shared(a) 
    a = this->parallel_task(half_a);
    #pragma omp task untied shared(b)
    b = this->parallel_task(half_b);
    #pragma omp taskwait

    spdlog::debug("Merging: {} and {}", a.graph_ids, b.graph_ids);

    GmSolution solution        = details::match(a, b, (*this->model));
    new_manager  = details::merge(a, b, solution, (*this->model));
    

    return new_manager;
}

namespace details {
    
GmSolution match(const CliqueManager& manager_1, const CliqueManager& manager_2, const MgmModel& model){
    spdlog::info("Matching...");
    CliqueMatcher matcher(manager_1, manager_2, model);
    return matcher.match();
}

//FIXME: could also be done inplace into manager_1
CliqueManager merge(const CliqueManager& manager_1, const CliqueManager& manager_2, const GmSolution& solution, const MgmModel& model) {
    spdlog::info("Merging...");

    // Prepare new clique_manager
    auto& g_m1 = manager_1.graph_ids;
    auto& g_m2 = manager_2.graph_ids;

    int size = g_m1.size() + g_m2.size();
    std::vector<int> merged_graph_ids(size);

    std::merge(g_m1.begin(), g_m1.end(), g_m2.begin(), g_m2.end(), merged_graph_ids.begin());

    CliqueManager new_manager(merged_graph_ids, model);
    new_manager.cliques.reserve(manager_1.cliques.no_cliques + manager_2.cliques.no_cliques);

    // Labeling indicates which clique of manager_1 should be merged with a clique in manager_2.
    // Remember which cliques in manager_2 were assigned, as unassigned ones have to be added later.
    auto is_assigned = std::vector<bool>(manager_2.cliques.no_cliques, false);
    int clique_idx = 0;
    for (auto l : solution.labeling) {
        auto new_clique = manager_1.cliques[clique_idx];
        if (l >= 0) {
            assert(is_assigned[l] == false);
            is_assigned[l] = true;

            new_clique.insert(manager_2.cliques[l].begin(), manager_2.cliques[l].end());
            assert(new_clique.size() <= (size_t) new_manager.cliques.no_graphs);
        }
        new_manager.cliques.add_clique(new_clique);
        clique_idx++;
    }

    // Add remaining cliques (May happen in parallel local search mode, when outdated solution has less labels than current manager's no_cliques)
    while (clique_idx < manager_1.cliques.no_cliques) {
        auto new_clique = manager_1.cliques[clique_idx];
        new_manager.cliques.add_clique(new_clique);
        clique_idx++;
    }

    // Add cliques from manager_2 that remain unassigned
    int clique_m_2 = 0;
    for (const auto& b : is_assigned) {
        if (!b){
            new_manager.cliques.add_clique(manager_2.cliques[clique_m_2]);
        }

        clique_m_2++;
    }
    
    new_manager.build_clique_idx_view();
    return new_manager;
}

std::pair<CliqueManager, CliqueManager> split(const CliqueManager &manager, int graph_id, const MgmModel& model) {
    
    CliqueManager manager_1(manager);
    manager_1.remove_graph(graph_id);

    CliqueManager manager_2(model.graphs[graph_id]);

    return std::make_pair(manager_1, manager_2);
}

CliqueMatcher::CliqueMatcher(const CliqueManager& manager_1, const CliqueManager& manager_2, const MgmModel& model)
    : manager_1(manager_1), manager_2(manager_2), model(model) {
    
    int g1 = this->manager_1.graph_ids[0];
    int g2 = this->manager_2.graph_ids[0];

    GmModelIdx graph_pair_idx = (g1 < g2) ? GmModelIdx(g1, g2) : GmModelIdx(g2, g1);

    size_t approximate_no_assignments_max = this->model.models.at(graph_pair_idx)->no_assignments;
    size_t approximate_no_edges_max = this->model.models.at(graph_pair_idx)->no_edges;

    this->clique_assignments.reserve(approximate_no_assignments_max);
    this->clique_edges.reserve(approximate_no_edges_max);

    // this->assignment_idx_map.reserve(model->models.size());
    // for (const auto& m : model->models) {
    //     assignment_idx_map.emplace(m.first, std::vector<int>(m.second->no_assignments, -1));
    // }
}

GmSolution CliqueMatcher::match() {
    auto model = std::make_shared<GmModel>(this->construct_qap());

    spdlog::info("Constructing QAP solver...");
    QAPSolver solver(model);

    spdlog::info("Running QAP solver...");
    return solver.run();
}

GmModel CliqueMatcher::construct_qap() {
    spdlog::info("Collecting assignments...");
    this->collect_assignments();
    spdlog::info("Collecting edges...");
    this->collect_edges();
    spdlog::info("Constructing model...");
    GmModel m = this->construct_gm_model();

    return m;
}

void CliqueMatcher::collect_assignments() {
    // FIXME: Consider changing this iterating over clique pairs. Break if assignment does not exist.
    // In Python, this is the new implementation, but I have doubts, if this is in fact faster.

    // For all graph pairs
    for (const auto& g1 : this->manager_1.graph_ids) {
        for (const auto& g2 : this->manager_2.graph_ids) {
            bool is_sorted = (g1 < g2);
            GmModelIdx graph_pair_idx = is_sorted ? GmModelIdx(g1, g2) : GmModelIdx(g2, g1);

            auto m = this->model.models.at(graph_pair_idx);

            int clique_g1 = -1, clique_g2 = -1;

            // Iterate over all assignments
            for (const auto& a : m->assignment_list) {
                if (is_sorted) {
                    clique_g1 = this->manager_1.clique_idx(g1, a.first);
                    clique_g2 = this->manager_2.clique_idx(g2, a.second);
                }
                else {
                    clique_g1 = this->manager_1.clique_idx(g1, a.second);
                    clique_g2 = this->manager_2.clique_idx(g2, a.first);
                }
                assert(clique_g1 >= 0);
                assert(clique_g2 >= 0);
                double cost = m->costs->unary(a);

                // Store as an assignment between two cliques.
                // Other graph pairs with an assignment in the same cliques may add a cost later.
                CliqueAssignmentIdx clique_idx(clique_g1, clique_g2);
                this->clique_assignments[clique_idx].push_back(cost);

                //assert(this->assignment_idx_map[graph_pair_idx].find(a) == this->assignment_idx_map[graph_pair_idx].end());
                //this->assignment_idx_map[graph_pair_idx][a] = clique_idx;
            }
        }
    }

    // remove invalid entries
    // FIXME: Avoidable loop?
    for (auto it = this->clique_assignments.begin(); it != this->clique_assignments.end();) {
        int clique1 = it->first.first;
        int clique2 = it->first.second;
        
        size_t len = it->second.size();
        size_t expected = this->manager_1.cliques[clique1].size() * this->manager_2.cliques[clique2].size();

        if(len < expected) {
            // at least one assigment has between the cliques has infinite cost and
            // should not be considered.
            it = this->clique_assignments.erase(it);
        }
        else {
            it++;
        }
    }
}

void CliqueMatcher::collect_edges() {
    // For all graph pairs
    for (const auto& g1 : this->manager_1.graph_ids) {
        for (const auto& g2 : this->manager_2.graph_ids) {
            bool is_sorted = (g1 < g2);
            GmModelIdx graph_pair_idx = is_sorted ? GmModelIdx(g1, g2) : GmModelIdx(g2, g1);

            auto m = this->model.models.at(graph_pair_idx);

            // Iterate over all edges
            for (const auto& [edge_idx, cost] : m->costs->all_edges()) {
                AssignmentIdx a1    = edge_idx.first;
                AssignmentIdx a2    = edge_idx.second;
                
                int clique_a1_n1;
                int clique_a1_n2;
                int clique_a2_n1;
                int clique_a2_n2;

                // Map assignment nodes onto cliques.
                if (is_sorted) {
                    clique_a1_n1 = this->manager_1.clique_idx(g1, a1.first);
                    clique_a1_n2 = this->manager_2.clique_idx(g2, a1.second);
                    clique_a2_n1 = this->manager_1.clique_idx(g1, a2.first);
                    clique_a2_n2 = this->manager_2.clique_idx(g2, a2.second);
                }
                else {
                    clique_a1_n1 = this->manager_1.clique_idx(g1, a1.second);
                    clique_a1_n2 = this->manager_2.clique_idx(g2, a1.first);
                    clique_a2_n1 = this->manager_1.clique_idx(g1, a2.second);
                    clique_a2_n2 = this->manager_2.clique_idx(g2, a2.first);
                }
                assert(clique_a1_n1 >= 0);
                assert(clique_a1_n2 >= 0);
                assert(clique_a2_n1 >= 0);
                assert(clique_a2_n2 >= 0);

                // Find the two pairs of cliques that the edge refers to.
                CliqueAssignmentIdx clique_a1(clique_a1_n1, clique_a1_n2);
                CliqueAssignmentIdx clique_a2(clique_a2_n1, clique_a2_n2);

                // Check if the two clique pairs are both present in the clique assignments.
                // (May have been removed due to infinity assignments between them)
                auto it_c1 = this->clique_assignments.find(clique_a1);
                auto it_c2 = this->clique_assignments.find(clique_a2);

                if ((it_c1 != this->clique_assignments.end()) && (it_c2 != this->clique_assignments.end())) {
                    EdgeIdx e(clique_a1, clique_a2);
                    this->clique_edges[e] += cost; // Default value-initializes to zero, according to standard.
                }
            }
        }
    }
}

GmModel CliqueMatcher::construct_gm_model() {

    int no_assignments = this->clique_assignments.size();
    int no_edges  = this->clique_edges.size();
    Graph g1(-1, this->manager_1.cliques.no_cliques);
    Graph g2(-1, this->manager_2.cliques.no_cliques);

    GmModel m(g1, g2, no_assignments, no_edges);

    spdlog::info("Constructing QAP: no_assignments: {}, no_edges:{} ", no_assignments,no_edges);
    
    int id = 0;
    for (const auto& [clique_idx, costs] : clique_assignments) {
        double cost = std::reduce(costs.begin(), costs.end());
        m.add_assignment(id, clique_idx.first, clique_idx.second, cost);
        id++;
    }

    for (const auto& [edge_idx, cost] : clique_edges) {
        auto & a1 = edge_idx.first;
        auto & a2 = edge_idx.second;
        m.add_edge(a1.first, a1.second, a2.first, a2.second, cost);
        id++;
    }

    return m;
}  
}
}

unsigned int largest_power_of_2_in(unsigned int input) {
    if (input == 0) 
        return 0;

    int n = 1;
    // bit shift to the right.
    // Try to find most significant bit
    while (input >>= 1) 
    {
        n*=2;
    }

    return n;
}