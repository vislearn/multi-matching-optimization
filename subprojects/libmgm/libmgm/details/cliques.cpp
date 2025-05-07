#include <cassert>
#include <vector>
#include <stdexcept>
#include <spdlog/spdlog.h>
#include <numeric>

#include "cliques.hpp"
#include "multigraph.hpp"

namespace mgm {
    
//FIXME: Clique is not Hashmap, not vector.
// Consider changing no_graphs to "max_no_graphs" to emphasize a rehash avoidance.
CliqueTable::CliqueTable(int no_graphs) {
    this->no_graphs = no_graphs;
    this->empty_clique = Clique(this->no_graphs);
}

void CliqueTable::add_clique() {
    this->cliques.push_back(empty_clique);
    this->no_cliques++;
}

// FIXME: Maybe make this a move function
void CliqueTable::add_clique(Clique c) {
    this->cliques.push_back(c);
    this->no_cliques++;
}

void CliqueTable::reserve(int no_cliques) {
    this->cliques.reserve(this->cliques.size() + no_cliques);
}

void CliqueTable::remove_graph(int graph_id, bool should_prune) {
    this->no_graphs--;
    this->empty_clique.reserve(this->no_graphs);
    
    for (auto& c : this->cliques) {
        c.erase(graph_id);
    }

    if (should_prune) {
        this->prune();
    }
}

void CliqueTable::prune() {
    for (auto it = this->cliques.begin(); it != this->cliques.end();) {
        if (it->empty()) {
            it = this->cliques.erase(it);
        }
        else {
            it++;
        }
    }
    this->no_cliques = this->cliques.size();
}

int& CliqueTable::operator()(int clique_id, int graph_id) {
    return this->cliques.at(clique_id)[graph_id];
}

const int& CliqueTable::operator()(int clique_id, int graph_id) const {
    return this->cliques.at(clique_id).at(graph_id);
}

CliqueTable::Clique& CliqueTable::operator[](int clique_id) {
    return this->cliques.at(clique_id);
}

const CliqueTable::Clique& CliqueTable::operator[](int clique_id) const {
    return this->cliques.at(clique_id);
}


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

CliqueManager::CliqueManager(std::vector<int> graph_ids, const MgmModel& model) 
        : cliques(graph_ids.size()), graph_ids(graph_ids) {
    for (auto& id : graph_ids) {
        this->clique_idx_view[id] = std::vector<int>(model.graphs[id].no_nodes, -1);
    }
}

CliqueManager::CliqueManager(std::vector<int> graph_ids, const MgmModel &model, CliqueTable table) 
    : CliqueManager(graph_ids, model) {
    this->cliques = table;
    this->build_clique_idx_view();
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
}