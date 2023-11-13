#include <map>
#include <vector>
#include <memory>
#include <utility>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <cassert>
#include <numeric>

// Logging
#include <spdlog/spdlog.h>
#include <fmt/ranges.h> // print vector

#include "multigraph.hpp"
#include "solution.hpp"
#include "cliques.hpp"
#include "solver_mgm.hpp"

namespace mgm {

constexpr double INFINITY_COST = 1e99;

GmSolution::GmSolution(std::shared_ptr<GmModel> model) {
    this->model = model;
    this->labeling = std::vector<int>(model->graph1.no_nodes, -1);
}

bool GmSolution::is_active(AssignmentIdx assignment) const {
    return this->labeling[assignment.first] == assignment.second;
}

double GmSolution::evaluate() const {

    double result = 0.0;

    // assignments
    int node = 0;
    for (const auto& label : this->labeling) {
        if (label >= 0) {
            if (this->model->costs->contains(node, label)) {
                result += this->model->costs->unary(node, label);
            }
            else {
                return INFINITY_COST;
            }
        }
        node++;
    }

    //edges
    for (const auto& [edge_idx, cost] : this->model->costs->all_edges()) {
        auto& a1 = edge_idx.first;
        auto& a2 = edge_idx.second;
        if (this->is_active(a1) && this->is_active(a2)) {
            result += cost;
        }
    }

    return result;
}


MgmSolution::MgmSolution(std::shared_ptr<MgmModel> model) {
    this->model = model;
    gmSolutions.reserve(model->models.size());

    for (auto const& [key, m] : model->models) {
        gmSolutions[key] = GmSolution(m);
    }
}

void MgmSolution::build_from(const CliqueTable& cliques)
{
    for (const auto& c : cliques) {
        for (const auto& [g1, n1] : c) {
            for (const auto& [g2, n2] : c) {
                if (g1 == g2) 
                    continue;
                if (g1 < g2) {
                    this->gmSolutions[GmModelIdx(g1,g2)].labeling[n1] = n2;
                }
                else {
                    this->gmSolutions[GmModelIdx(g2,g1)].labeling[n2] = n1;
                }
            }
        }
    }
}

CliqueTable MgmSolution::export_cliquetable(){
    CliqueTable table(this->model->no_graphs);

    // 2d array to store clique_idx of every node
    // node_clique_idx[graph_id][node_id] = clique_idx of table
    std::vector<std::vector<int>> node_clique_idx;
    for (const auto& g : this->model->graphs) {
        node_clique_idx.emplace_back(g.no_nodes, -1);
    }
    
    for (int g1 = 0; g1 < this->model->no_graphs; g1++){
        for (int g2 = (g1 + 1); g2 < this->model->no_graphs; g2++) {
            GmModelIdx model_idx(g1,g2);

            if (this->gmSolutions.find(model_idx) == this->gmSolutions.end())
                continue;

            const auto& l = this->gmSolutions[model_idx].labeling;
            
            for (size_t node_id = 0; node_id < l.size(); node_id++) {
                const int & label = l[node_id];
                if (label < 0) 
                    continue;
                
                int& clique_idx_n1 = node_clique_idx[g1][node_id];
                int& clique_idx_n2 = node_clique_idx[g2][label];
                if (clique_idx_n1 < 0 && clique_idx_n2 < 0) {
                    CliqueTable::Clique c;
                    c[g1] = node_id;
                    c[g2] = label;
                    table.add_clique(c);
                    clique_idx_n1 = table.no_cliques-1;
                    clique_idx_n2 = table.no_cliques-1;
                }
                else if (clique_idx_n1 >= 0 && clique_idx_n2 < 0) {
                    table[clique_idx_n1][g2] = label;
                    clique_idx_n2 = clique_idx_n1;
                }
                else if (clique_idx_n1 < 0 && clique_idx_n2 >= 0) {
                    table[clique_idx_n2][g1] = node_id;
                    clique_idx_n1 = clique_idx_n2;
                }
                else if (clique_idx_n1 != clique_idx_n2) {
                    throw std::logic_error("Cycle inconsistent labeling given. Nodes matched two each other implied to reside in different cliques because of previously parsed matchings.");
                }
            }
        }
    }
    
    // Add all remaining, unmatched nodes in a clique of their own.
    for (size_t graph_id = 0; graph_id < node_clique_idx.size(); graph_id++) {
        for (size_t node_id = 0; node_id < node_clique_idx[graph_id].size(); node_id++) {
            if (node_clique_idx[graph_id][node_id] >= 0)
                continue;
            CliqueTable::Clique c;
            c[graph_id] = node_id;
            table.add_clique(c);
        }
    }

    #ifndef DEBUG
    // VERIFIES THAT CLIQUES WERE PARSED CORRECTLY
    std::vector<int> graph_ids(this->model->no_graphs);
    std::iota(graph_ids.begin(), graph_ids.end(), 0);
    
    CliqueManager cm(graph_ids, (*this->model));
    cm.reconstruct_from(table);

    for (const auto& [model_idx, gm_solution] : this->gmSolutions) {
        const auto& l = gm_solution.labeling;
        for (size_t node_id = 0; node_id < l.size(); node_id++) {
            const int & label = l[node_id];
            if (label < 0)
                continue;
            assert(cm.clique_idx(model_idx.first, node_id) == cm.clique_idx(model_idx.second, label));
        }
    }
    #endif

    return table;
}

double MgmSolution::evaluate() const {
    double result = 0.0;
    for (const auto& m : this->gmSolutions) {
        result += m.second.evaluate();
    }
    return result;
}

bool MgmSolution::is_cycle_consistent() const{
    return true;
}

}