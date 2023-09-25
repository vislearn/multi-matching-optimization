#include <cassert>
#include <vector>
#include <stdexcept>
#include <spdlog/spdlog.h>

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

std::unordered_map<int, int>& CliqueTable::operator[](int clique_id) {
    return this->cliques.at(clique_id);
}

const std::unordered_map<int, int>& CliqueTable::operator[](int clique_id) const {
    return this->cliques.at(clique_id);
}

}