#include <algorithm>
#include <cassert>
#include <vector>
#include <sstream>

#include <spdlog/spdlog.h>

#include <qpbo.h>

#include "cliques.hpp"
#include "solver_local_search_swap.hpp"
#include "solution.hpp"

constexpr double INFINITY_COST = 1e99;
constexpr double QPBO_ENERGY_THRESHOLD = -0.000001;

namespace mgm {

SwapLocalSearcher::SwapLocalSearcher(std::shared_ptr<MgmModel> model)
    : model(model) {}

bool SwapLocalSearcher::search(MgmSolution& input) {
    assert(input.clique_table().no_cliques > 1);
    spdlog::info("Optimizing using alpha beta swap.\n");

    // creates a copy here once. Modified using references everywhere else and moved back later into the MgmSolution object.
    this->current_state = input.clique_table();

    // New search. Check all cliques
    this->reset();
    bool search_improved = false;
    bool iteration_improved = true;
    double initial_energy = input.evaluate();

    this->clique_optimizer = std::make_unique<details::CliqueSwapper>(  this->model->no_graphs,
                                                                        this->model, 
                                                                        this->current_state,
                                                                        this->max_iterations_QPBO_I);

    while (iteration_improved) {
        spdlog::info("Current energy: {}", initial_energy);

        iteration_improved = this->iterate();

        if (iteration_improved)
            search_improved = true;

        if (this->current_step >= this->max_iterations) {
            spdlog::info("Iteration limit reached. Stopping after {} iterations.", this->current_step);
            return search_improved;
        }
    }
    spdlog::info("No change through previous iteration. Stopping after {} iterations.", this->current_step);

    double final_energy = 0;
    if (search_improved) {
        input.set_solution(std::move(this->current_state));
        final_energy = input.evaluate();
    }
    else {
        final_energy = initial_energy;
    }
    spdlog::info("Finished swap local search. Current energy: {}\n", final_energy);

    return search_improved;
}

void SwapLocalSearcher::reset() {
    this->current_step = 0;
    this->cliques_changed_prev.assign(  this->current_state.no_cliques, true);
    this->cliques_changed.assign(       this->current_state.no_cliques, false);
}

bool SwapLocalSearcher::iterate()
{
    this->current_step++;
    bool improved = false;

    std::vector<CliqueTable::Clique> new_cliques;

    spdlog::info("Iteration {}", this->current_step);
    spdlog::info("No of Cliques: {}", this->current_state.no_cliques);
    
    // Every clique
    int idx_A = 0;
    bool print_a = true;
    for (auto & clique_A : this->current_state) {
        // To all cliques after clique_A
        for (auto it = this->current_state.begin() + (idx_A + 1); it != this->current_state.end(); it++) {
            int idx_B = it - this->current_state.begin();
            auto & clique_B = *it;

            // Skip if both cliques haven't changed in previous iteration
            if (!(this->cliques_changed_prev[idx_A] || this->cliques_changed_prev[idx_B])) 
                continue;

            // No need to compare to empty clique
            if (this->current_state[idx_B].empty())
                continue;

            if (print_a) {
                spdlog::info("Clique {} / {}", idx_A+1, this->current_state.no_cliques);
                print_a = false;
            }

            bool should_flip = this->clique_optimizer->optimize(clique_A, clique_B);

            if (should_flip && this->clique_optimizer->current_solution.energy < QPBO_ENERGY_THRESHOLD) {
                improved = true;

                this->cliques_changed[idx_A] = true;
                this->cliques_changed[idx_B] = true;

            #ifndef NDEBUG
                auto s = MgmSolution(this->model);
                s.set_solution(this->current_state);
                double e_prior = s.evaluate();
                spdlog::debug("Energy before flip: {}", e_prior);
                double e_qpbo = clique_optimizer->current_solution.energy;
                spdlog::debug("QPBO Energy: {}", e_qpbo);
            #endif

                details::flip(clique_A, clique_B, this->clique_optimizer->current_solution);

            #ifndef NDEBUG
                s = MgmSolution(this->model);
                s.set_solution(this->current_state);
                double e_after = s.evaluate();
                spdlog::debug("Energy After flip: {}", e_after);
                spdlog::debug("Should be: {}", e_prior + e_qpbo);
                spdlog::debug("Difference: {}", ((e_prior + e_qpbo) - e_after));
            #endif

                if (clique_A.empty()) {
                    break;
                }
            }
        }
        if (this->cliques_changed_prev[idx_A]) {
            //Special Case: Compare with empty clique
            //spdlog::info("Checking against empty clique...");
            bool improved = this->clique_optimizer->optimize_with_empty(clique_A);
                if (improved) {
                    spdlog::info("Improvement found. Splitting clique {}.", idx_A);

                    this->cliques_changed[idx_A] = true;

                    auto new_clique = CliqueTable::Clique();
                    details::flip(clique_A, new_clique, this->clique_optimizer->current_solution);

                    new_cliques.push_back(new_clique);

                    assert(!clique_A.empty());
                }
        }
        idx_A++;
        print_a = true;
    }
    post_iterate_cleanup(new_cliques);
    return improved;
}

void SwapLocalSearcher::post_iterate_cleanup(std::vector<CliqueTable::Clique>& new_cliques)
{   
    // Safe which cliques changed for next iteration.
    // Skip empty cliques, as they will be removed.
    int i_changed_prev = 0;
    for (size_t i = 0; i < this->cliques_changed.size(); i++) {
        if (this->current_state[i].empty()) {
            continue;
        }
        else {
            // only advance iterator on changed prev for non-empty cliques.
            this->cliques_changed_prev[i_changed_prev] = this->cliques_changed[i];
            i_changed_prev++;
        }
    }
    // Remove any empty cliques
    this->current_state.prune();
    this->cliques_changed_prev.resize(this->current_state.no_cliques);

    // Add new cliques 
    // Mark as changed cliques for next iteration, so they will be considered for swapping.
    for (const auto & c : new_cliques) {
        this->current_state.add_clique(c);
    }
    this->cliques_changed_prev.resize(this->current_state.no_cliques, true);

    // reset
    this->cliques_changed.assign(this->current_state.no_cliques, false);
}

namespace details{

std::vector<int> unique_keys(CliqueTable::Clique &A, CliqueTable::Clique &B, int num_graphs);

CliqueSwapper::CliqueSwapper(int num_graphs, std::shared_ptr<MgmModel> model, CliqueTable& current_state, int max_iterations_QPBO_I) 
    :   qpbo_solver(num_graphs, ((num_graphs*num_graphs) / 2)),
        model(model),
        current_state(current_state),
        max_iterations_QPBO_I(max_iterations_QPBO_I) {}


bool CliqueSwapper::optimize(CliqueTable::Clique &A, CliqueTable::Clique &B)
{
    auto & graphs = this->current_solution.graphs; // alias

    // Get unique graphs currently present in both cliques.
    graphs = unique_keys(A, B, this->model->no_graphs);
    const auto groups = build_groups(graphs, A, B, this->model);
    this->current_solution.groups = groups;

    int no_nodes = groups.size();
    if (no_nodes < 2) {
        return false; // nothing to optimize. 
    }
    
    // Initialize number of nodes
    this->qpbo_solver.Reset();
    qpbo_solver.AddNode(no_nodes);

    // Add unary costs
    for (int i = 0; i < no_nodes; i++) {
        qpbo_solver.AddUnaryTerm(i, 0, 0);
        qpbo_solver.SetLabel(i, 0);
    }

    // Add edge costs
    int idx_group1 = 0;
    for (const auto & group1 : groups) {
        int idx_group2 = idx_group1 + 1;
        for (auto it = groups.begin() + (idx_group1 + 1); it != groups.end(); it++) {
            const auto & group2 = *it;
            double cost = 0.0;

            for (const int & g1 : group1) {
                // Assign node-id if graph is contained in clique
                // Otherwise assign -1 to indicate graph is not in clique.
                auto alpha1_it   = A.find(g1);
                auto beta1_it    = B.find(g1);
                auto alpha1   = (alpha1_it != A.end())  ? alpha1_it->second : -1;
                auto beta1    = (beta1_it != B.end())   ? beta1_it->second  : -1;

                for (const int & g2 : group2) {
                    auto alpha2_it   = A.find(g2);
                    auto beta2_it    = B.find(g2);
                    auto alpha2   = (alpha2_it != A.end())  ? alpha2_it->second : -1;
                    auto beta2    = (beta2_it != B.end())   ? beta2_it->second  : -1;

                    if (g1 < g2) {
                        cost += star_flip_cost(g1, g2, alpha1, alpha2, beta1, beta2);
                    }
                    else {
                        cost += star_flip_cost(g2, g1, alpha2, alpha1, beta2, beta1);
                    }
                }
            }

            qpbo_solver.AddPairwiseTerm(idx_group1, idx_group2, 0, cost, cost, 0);
            idx_group2++;
        }
        idx_group1++;
    }

    this->current_solution.improved = run_qpbo_solver();
    return this->current_solution.improved;
}

bool CliqueSwapper::optimize_with_empty(CliqueTable::Clique &A)
{
    auto empty = CliqueTable::Clique();
    return this->optimize(A, empty);
}

bool CliqueSwapper::optimize_no_groups(CliqueTable::Clique &A, CliqueTable::Clique &B)
{
    this->qpbo_solver.Reset();
    auto & graphs = this->current_solution.graphs; // alias

    // Get unique graphs currently present in both cliques.
    graphs = unique_keys(A, B, this->model->no_graphs);

    // Initialize number of nodes
    int no_nodes = graphs.size();
    qpbo_solver.AddNode(no_nodes);

    // Add unary costs
    for (int i = 0; i < no_nodes; i++) {
        qpbo_solver.AddUnaryTerm(i, 0, 0);
        qpbo_solver.SetLabel(i, 0);
    }

    // Add edge costs
    int idx_g1 = 0;
    for (const auto & g1 : this->current_solution.graphs) {
        int idx_g2 = idx_g1 + 1;
        for (auto it = graphs.begin() + (idx_g1 + 1); it != graphs.end(); it++) {
            
            auto & g2 = *it;
            
            // Assign node-id if graph is contained in clique
            // Otherwise assign -1 to indicate graph is not in clique.
            auto alpha1_it   = A.find(g1);
            auto alpha2_it   = A.find(g2);
            auto beta1_it    = B.find(g1);
            auto beta2_it    = B.find(g2);

            auto alpha1   = (alpha1_it != A.end())  ? alpha1_it->second : -1;
            auto alpha2   = (alpha2_it != A.end())  ? alpha2_it->second : -1;
            auto beta1    = (beta1_it != B.end())   ? beta1_it->second  : -1;
            auto beta2    = (beta2_it != B.end())   ? beta2_it->second  : -1;

            double cost = star_flip_cost(g1, g2, alpha1, alpha2, beta1, beta2);
            qpbo_solver.AddPairwiseTerm(idx_g1, idx_g2, 0, cost, cost, 0);

            idx_g2++;
        }
        idx_g1++;
    }

    this->current_solution.improved = run_qpbo_solver();

    return this->current_solution.improved;
}

bool CliqueSwapper::optimize_with_empty_no_groups(CliqueTable::Clique &A)
{
    auto empty = CliqueTable::Clique();
    return this->optimize_no_groups(A, empty);
}

bool CliqueSwapper::run_qpbo_solver()
{
    bool success = false;

    // Run till improvement
    for (int i = 0; i < this->max_iterations_QPBO_I; i++) {
        if (this->qpbo_solver.Improve()) {
            success = true;
            break;
        }
    }
    int node_num = qpbo_solver.GetNodeNum();
    this->current_solution.flip_indices.assign(node_num,0);

    for (int i = 0; i < node_num; i++) {
        if (qpbo_solver.GetLabel(i) == 1) {
            this->current_solution.flip_indices[i] = 1;
        }
    }
    this->current_solution.energy = qpbo_solver.ComputeTwiceEnergy() / 2;
    return success;
}

inline double get_flip_cost(AssignmentIdx& a, const mgm::AssignmentContainer & assignments) {
    auto a_it = assignments.find(a);
    return (a_it != assignments.end() ? a_it->second : INFINITY_COST);
}

inline EdgeIdx construct_sorted(AssignmentIdx& a, AssignmentIdx& b) {
    return (a.first < b.first) ? EdgeIdx(a, b): EdgeIdx(b, a);
}

double CliqueSwapper::star_flip_cost(int id_graph1, int id_graph2, int alpha1, int alpha2, int beta1, int beta2)
{
    const auto& m = this->model->models.at(GmModelIdx(id_graph1, id_graph2));
    double cost = 0.0;

    auto old_assignment_1 = AssignmentIdx(alpha1, alpha2);
    auto old_assignment_2 = AssignmentIdx(beta1, beta2);

    auto new_assignment_1 = AssignmentIdx(alpha1, beta2);
    auto new_assignment_2 = AssignmentIdx(beta1, alpha2);

    auto& assignments = m->costs->all_assignments();
    if (alpha1 != -1) {
        if(alpha2 != -1) 
            cost -= get_flip_cost(old_assignment_1, assignments);
        if(beta2 != -1)
            cost += get_flip_cost(new_assignment_1, assignments);
    }
    if(beta1 != -1) {
        if(beta2 != -1)
            cost -= get_flip_cost(old_assignment_2, assignments);
        if(alpha2 != -1)
            cost += get_flip_cost(new_assignment_2, assignments);
    }
    
    // pairwise
    auto& edges = m->costs->all_edges();
    for (const auto & c : this->current_state) {
        auto g1_it = c.find(id_graph1);
        if(g1_it == c.end())
            continue;

        auto g2_it = c.find(id_graph2);
        if(g2_it == c.end())
            continue;

        AssignmentIdx pair(g1_it->second, g2_it->second);
        if(old_assignment_1 == pair || old_assignment_2 == pair)
            continue;

        auto old_edge_1 = construct_sorted(old_assignment_1, pair);
        auto old_edge_2 = construct_sorted(old_assignment_2, pair);
        auto new_edge_1 = construct_sorted(new_assignment_1, pair);
        auto new_edge_2 = construct_sorted(new_assignment_2, pair);

        auto e_it = edges.find(old_edge_1);
        if(e_it != edges.end())
            cost -= e_it->second;

        e_it = edges.find(old_edge_2);
        if(e_it != edges.end())
            cost -= e_it->second;

        e_it = edges.find(new_edge_1);
        if(e_it != edges.end())
            cost += e_it->second;

        e_it = edges.find(new_edge_2);
        if(e_it != edges.end())
            cost += e_it->second;
    }

    // account for edge between old and new assignments.
    auto old_edge = construct_sorted(old_assignment_1, old_assignment_2);
    auto new_edge = construct_sorted(new_assignment_1, new_assignment_2);

    auto e_it = edges.find(old_edge);
    if(e_it != edges.end())
        cost -= e_it->second;

    e_it = edges.find(new_edge);
    if(e_it != edges.end())
        cost += e_it->second;

    return cost;
}

// return SORTED set_union over clique A and clique B keys (keys=graph_id)
// num_graphs added for convinience to estimate max size of the returned array.
std::vector<int> unique_keys(CliqueTable::Clique& A, CliqueTable::Clique& B, int num_graphs) {
    std::vector<bool> is_present(num_graphs, false);

    for (const auto& [k,v] : A) {
        is_present[k] = true;
    }
    for (const auto& [k,v] : B) {
        is_present[k] = true;
    }

    std::vector<int> merged_keys;
    merged_keys.reserve(num_graphs);

    for(int i = 0; i < num_graphs; i++) {
        if(is_present[i])
            merged_keys.push_back(i);
    }

    return merged_keys;
}

void flip(CliqueTable::Clique &A, CliqueTable::Clique &B, CliqueSwapper::Solution & solution) {

    for (size_t i = 0; i < solution.flip_indices.size(); i++) {
        if (solution.flip_indices[i] == 0)
            // no flip
            continue;

        // Should flip
        for (const auto & graph_id : solution.groups[i]) {
            auto a_entry = A.find(graph_id);
            auto b_entry = B.find(graph_id);
        
            if (a_entry != A.end() && b_entry != B.end()) {
                // Both cliques contain graph. Swap entries
                std::swap(a_entry->second, b_entry->second);
            }
            else if (a_entry != A.end())
            {
                // Only clique A contians graph. Transfer node over to B.
                B[a_entry->first] = a_entry->second;
                A.erase(a_entry);
            }
            else if (b_entry != B.end())
            {
                // Only clique B contians graph. Transfer node over to A.
                A[b_entry->first] = b_entry->second;
                B.erase(b_entry);
            }
            else {
                throw std::logic_error("At least one clique should contain the graph_id");
            }
        }
    }
}

struct SwapGroupManager {
    std::vector<SwapGroup> groups;
    std::unordered_map<int, std::size_t> graph_to_group;
    std::size_t group_count;

    explicit SwapGroupManager(const std::vector<int>& graphs) {
        group_count = graphs.size();
        groups.reserve(group_count);
        for (std::size_t i = 0; i < group_count; ++i) {
            groups.emplace_back();

            groups.back().reserve(group_count);
            groups.back().push_back(graphs[i]);
            graph_to_group[graphs[i]] = i;
        }
    }
    void merge(std::size_t idx1, std::size_t idx2) {
        assert (idx1 != idx2);

        for (const auto& graph_idx : groups[idx2]) {
            graph_to_group[graph_idx] = idx1;
        }
        groups[idx1].insert(groups[idx1].end(), groups[idx2].begin(), groups[idx2].end());
        groups[idx2].clear();
        --group_count;
    }
};

bool should_merge(const int g1, const SwapGroup& group, const CliqueTable::Clique& A, const CliqueTable::Clique& B, std::shared_ptr<MgmModel> model) {
    auto alpha1_it = A.find(g1);
    auto beta1_it  = B.find(g1);

    for (const auto & g2 : group) {
        auto alpha2_it = A.find(g2);
        auto beta2_it  = B.find(g2);

        // Edge case for clique A/B not containing a vertex of g1/g2.
        bool a1_exists = alpha1_it != A.end() && beta2_it != B.end();
        bool a2_exists = beta1_it != B.end() && alpha2_it != A.end();

        if (g1 < g2){
            const auto& m = model->models.at(GmModelIdx(g1, g2));

            if ((a1_exists && !m->costs->contains(alpha1_it->second, beta2_it->second)) ||
                (a2_exists && !m->costs->contains(beta1_it->second, alpha2_it->second))) {
                return true;
            }
        }
        else{
            const auto& m = model->models.at(GmModelIdx(g2, g1));

            if ((a1_exists && !m->costs->contains(beta2_it->second, alpha1_it->second)) ||
                (a2_exists && !m->costs->contains(alpha2_it->second, beta1_it->second))) {
                return true;
            }
        }
    }
    return false;
}

std::vector<SwapGroup> prune_empty(std::vector<SwapGroup>&& groups) {
    std::vector<SwapGroup> pruned_groups;
    pruned_groups.reserve(groups.size());
    for (auto& group : groups) {
        if (!group.empty()) {
            pruned_groups.push_back(std::move(group));
        }
    }
    return pruned_groups;
}

std::vector<SwapGroup> build_groups(const std::vector<int>& graphs, const CliqueTable::Clique& A, const CliqueTable::Clique& B, const std::shared_ptr<MgmModel> model) {
    SwapGroupManager mgr(graphs);

    for (const auto & current_graph : graphs) {
        if (mgr.group_count == 1) break;

        std::size_t current_idx = mgr.graph_to_group[current_graph];

        for (std::size_t other_idx = 0; other_idx < mgr.groups.size(); ++other_idx) {
            if (mgr.groups[other_idx].empty() || other_idx == current_idx) continue;

            if (should_merge(current_graph, mgr.groups[other_idx], A, B, model)) {
                mgr.merge(current_idx, other_idx);
            }
        }
    }
    return prune_empty(std::move(mgr.groups));
};

}
}

