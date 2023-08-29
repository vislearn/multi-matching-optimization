#include <memory>
#include <vector>
#include <unordered_map>
#include <queue>
#include <exception>
#include <algorithm>
#include <random>
#include <cassert>
#include <numeric>

#include "cliques.hpp"
#include "multigraph.hpp"
#include "qap_interface.hpp"

#include "solver_mgm.hpp"

CliqueManager::CliqueManager(Graph g) : cliques(1) {
    this->graph_ids.push_back(g.id);
    this->clique_idx_view[g.id] = std::vector<int>(g.no_nodes, -1);
}

CliqueManager::CliqueManager(std::vector<int> graph_ids, const MgmModel& model) : cliques(graph_ids.size()) {
    this->graph_ids = graph_ids;
    for (auto& id : graph_ids) {
        this->clique_idx_view[id] = std::vector<int>(model.graphs[id].no_nodes, -1);
    }
}

int CliqueManager::clique_idx(int graph_id, int node_id) {
    return this->clique_idx_view.at(graph_id).at(node_id);
}

const int& CliqueManager::clique_idx(int graph_id, int node_id) const {
    return this->clique_idx_view.at(graph_id).at(node_id);
}

void CliqueManager::rebuild_clique_idx_view() {}


MgmGenerator::MgmGenerator(std::shared_ptr<MgmModel> model) : model(model) {

}

void MgmGenerator::generate(generation_order order) {
    init_generation_queue(order);

    // Move first entry in queue to current_state.
    this->current_state = std::make_unique<CliqueManager>(std::move(this->generation_queue.front()));
    this->generation_queue.pop();

    while (!this->generation_queue.empty()) {
        CliqueManager& current = (*this->current_state.get());
        CliqueManager& next = this->generation_queue.front();

        this->current_state = std::make_unique<CliqueManager>(merge(current, next));
        this->generation_queue.pop();
    }
}

void MgmGenerator::step() {
    assert(!this->generation_queue.empty());
    CliqueManager& current = (*this->current_state.get());
    CliqueManager& next = this->generation_queue.front();

    GmSolution solution = this->match(current, next);
    this->current_state = std::make_unique<CliqueManager>(merge(current, next, solution));
}

GmSolution MgmGenerator::match(const CliqueManager& manager_1, const CliqueManager& manager_2){
    CliqueMatcher matcher(manager_1, manager_2, this->model);
    return matcher.match();
}

//FIXME: could also be done inplace into manager_1
CliqueManager MgmGenerator::merge(const CliqueManager& manager_1, const CliqueManager& manager_2, const GmSolution& solution) const{

    // Prepare new clique_manager
    auto& g_m1 = manager_1.graph_ids;
    auto& g_m2 = manager_2.graph_ids;

    int size = g_m1.size() + g_m2.size();
    std::vector<int> merged_graph_ids(size);

    std::merge(g_m1.begin(), g_m1.end(), g_m2.begin(), g_m2.end(), merged_graph_ids.begin());

    CliqueManager new_manager(merged_graph_ids, (*this->model));

    // Fill Clique Table with old values from manager_1
    new_manager.cliques.reserve(manager_1.cliques.no_cliques);
    for (auto c : manager_1.cliques) {
        c.resize(size, -1);
        new_manager.cliques.add_clique(c);
    }

    //


    new_manager.rebuild_clique_idx_view();
    return new_manager;
}

void MgmGenerator::init_generation_queue(generation_order order) {

    // generate sequential order
    std::vector<int> ordering(this->model->no_graphs);
    std::iota(ordering.begin(), ordering.end(), 0);
    
    // shuffle if order should be random
    if (order == random) {
        std::shuffle(ordering.begin(), ordering.end(), std::random_device());
    }

    for (const auto& id : ordering) {
        Graph& g = this->model->graphs[id];
        this->generation_queue.push(CliqueManager(g));
    }
}

CliqueMatcher::CliqueMatcher(const CliqueManager& manager_1, const CliqueManager& manager_2,  std::shared_ptr<MgmModel> model)
    : manager_1(manager_1), manager_2(manager_2), model(model) {

        this->assignment_idx_map.reserve(model->models.size());
        for (const auto& m : model->models) {
            assignment_idx_map.emplace(m.first, std::vector<int>(m.second->no_assignments, -1));
        }
    }

GmSolution CliqueMatcher::match() {
    auto model = std::make_shared<GmModel>(this->construct_qap()); 
    QAPSolver solver(model);

    return solver.run();
}

GmModel CliqueMatcher::construct_qap() {
    this->collect_assignments();
    GmModel m = this->construct_gm_model();

    return m;
}

void CliqueMatcher::collect_assignments() {
    // FIXME: Consider changing this iterating over clique pairs. Break if assignment does not exist.
    // In Python, this is the new implementation, but I have doupts, if this is in fact faster.
    // For all graph pairs
    for (const auto& g1 : this->manager_1.graph_ids) {
        for (const auto& g2 : this->manager_2.graph_ids) {
            bool is_sorted = (g1 < g2);
            GmModelIdx graph_pair_idx = is_sorted ? GmModelIdx(g1, g2) : GmModelIdx(g2, g1);

            auto m = this->model->models[graph_pair_idx];

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
                double cost = m->costs->unary(a);

                // Store as an assignment between two cliques.
                // Other graph pairs with an assignment in the same cliques may add a cost later.
                CliqueAssignmentIdx clique_idx(clique_g1, clique_g2);
                this->clique_assignments[clique_idx].push_back(cost);

                assert(this->assignment_idx_map[graph_pair_idx].find(a) == this->assignment_idx_map[graph_pair_idx].end());
                this->assignment_idx_map[graph_pair_idx][a] = clique_idx;
            }
        }
    }
}

GmModel CliqueMatcher::construct_gm_model() {

    // These sizes are just approximations to reserve space.
    // Due to some clique assignments potentially being invalid due to not existing assignments (see below)
    // KEEP TRACK AND CHANGE THESE LATER ON, SUCH THAT RETURNED MODEL IS ACCURATE.
    int no_assignments = this->clique_assignments.size();
    int no_edges  = this->clique_edges.size();
    Graph g1(-1, this->manager_1.cliques.no_cliques);
    Graph g2(-1, this->manager_2.cliques.no_cliques);

    GmModel m(g1, g2, no_assignments, no_edges);

    // assignments
    int id = 0;
    for (const auto& a : clique_assignments) {
        size_t no_costs = a.second.size();

        int a_n1 = a.first.first;
        int a_n2 = a.first.second;
        size_t size_expected = this->manager_1.cliques[a_n1].size() * this->manager_2.cliques[a_n2].size();

        if (no_costs != size_expected) {
            // At least one node pair between both cliques has infinity costs.
            m.no_assignments--;
            continue;
        }
        auto cost = std::reduce(a.second.begin(),a.second.end());
        m.add_assignment(id, a_n1, a_n2, cost);
        this->assignment_ids[a.first] = id; // IMPORTANT FOR EDGES AFTERWARDS
        id++;
    }

    // edges
    for (const auto& g1 : this->manager_1.graph_ids) {
        for (const auto& g2 : this->manager_2.graph_ids) {
            bool is_sorted = (g1 < g2);
            GmModelIdx graph_pair_idx = is_sorted ? GmModelIdx(g1, g2) : GmModelIdx(g2, g1);

            auto m = this->model->models[graph_pair_idx];

            for (const auto& edge : m->costs->all_edges()) {
                AssignmentIdx a1    = edge.first.first;
                AssignmentIdx a2    = edge.first.second;
                double cost         = edge.second;
                
                int clique_a1_n1;
                int clique_a1_n2;
                int clique_a2_n1;
                int clique_a2_n2;

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
            }
        }
    }
}   

GmModel CliqueMatcher::new_construct_qap(){


    using ids = std::vector<std::vector<int>>;
    using costs = std::vector<double>;
    std::pair<ids, costs>;

    // collect assignments
    //assignments_
    //std::vector<int*> assignments(,nullptr);
    int c1_idx = 0, c2_idx = 0;
    for (const auto& clique1 : this->manager_1.cliques) {
        for (const auto& clique2 : this->manager_1.cliques) {
            
            for (const auto& c1_entry : clique1) {
                for (const auto& c2_entry : clique2) {
                    int g1 = c1_entry.first;
                    int g2 = c2_entry.first;
                    int g1_node = c1_entry.second;
                    int g2_node = c2_entry.second;

                    bool is_sorted = (g1 < g2);
                    GmModelIdx graph_pair_idx = is_sorted ? GmModelIdx(g1, g2) : GmModelIdx(g2, g1);

                    //auto& m = 
                }
            }

            c2_idx++;
        }
        c1_idx++;
    }
}