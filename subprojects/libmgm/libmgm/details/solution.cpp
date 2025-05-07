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

#include "multigraph.hpp"
#include "solution.hpp"
#include "cliques.hpp"


namespace mgm {

constexpr double INFINITY_COST = 1e99;
// Forward declaration
CliqueTable clique_table_from_labeling(Labeling& labeling, std::vector<Graph>& graphs);

GmSolution::GmSolution(std::shared_ptr<GmModel> model) : model(model) {
    this->labeling_ = std::vector<int>(model->graph1.no_nodes, -1);
}

GmSolution::GmSolution(std::shared_ptr<GmModel> model, std::vector<int> labeling) 
    : model(model), labeling_(labeling) {}

bool GmSolution::is_active(const AssignmentIdx& assignment, const std::vector<int> &labeling)
{
    return labeling[assignment.first] == assignment.second;
}

bool GmSolution::is_active(AssignmentIdx assignment) const {
    return GmSolution::is_active(assignment, this->labeling_);
}

double GmSolution::evaluate(const GmModel &model, const std::vector<int> &labeling)
{
    double result = 0.0;

    // assignments
    int node = 0;
    for (const auto& label : labeling) {
        if (label >= 0) {
            if (model.costs->contains(node, label)) {
                result += model.costs->unary(node, label);
            }
            else {
                return INFINITY_COST;
            }
        }
        node++;
    }

    //edges
    for (const auto& [edge_idx, cost] : model.costs->all_edges()) {
        auto& a1 = edge_idx.first;
        auto& a2 = edge_idx.second;
        if (GmSolution::is_active(a1, labeling) && GmSolution::is_active(a2, labeling)) {
            result += cost;
        }
    }

    return result;
}

double GmSolution::evaluate() const
{
    return GmSolution::evaluate((*this->model), this->labeling_);
}

int& GmSolution::operator[](int idx) {
    return this->labeling_[idx];
}

const int& GmSolution::operator[](int idx) const {
    return this->labeling_[idx];
}
const std::vector<int>& GmSolution::labeling() const{
    return this->labeling_;
}

std::vector<int>& GmSolution::labeling(){
    return this->labeling_;
}


MgmSolution::MgmSolution(std::shared_ptr<MgmModel> model) : model(model) {}

const Labeling& MgmSolution::labeling() const{
    if (this->labeling_valid) {
        return this->labeling_;
    }

    // Convert from clique table representation
    assert(this->clique_table_valid); 

    Labeling res = this->create_empty_labeling();

    for (const auto& c : this->ct) {
        for (const auto& [g1, n1] : c) {
            for (const auto& [g2, n2] : c) {
                if (g1 == g2) 
                    continue;
                if (g1 < g2) {
                    res[GmModelIdx(g1,g2)][n1] = n2;
                }
                else {
                    res[GmModelIdx(g2,g1)][n2] = n1;
                }
            }
        }
    }
    this->labeling_ = res;
    this->labeling_valid = true;
    return this->labeling_;
}

const CliqueManager& MgmSolution::clique_manager() const {
    if (this->clique_manager_valid) {
        return this->cm;
    }
    assert(this->clique_table_valid || this->labeling_valid);
    assert(this->model->no_graphs == this->clique_table().no_graphs);

    std::vector<int> graph_ids(this->model->no_graphs);
    std::iota(graph_ids.begin(), graph_ids.end(), 0);

    this->cm = CliqueManager(graph_ids, (*this->model), this->clique_table());
    this->clique_manager_valid = true;

    return this->cm;
}

const CliqueTable& MgmSolution::clique_table() const {
    if (this->clique_table_valid) {
        return this->ct;
    }
    assert(this->labeling_valid); // If CliqueTable is not valid, CliqueManager can not be valid either.

    auto res = clique_table_from_labeling(this->labeling_, this->model->graphs);

    this->ct = res;
    this->clique_table_valid = true;

    return this->ct;
}

void MgmSolution::set_solution(const Labeling &labeling) {
    this->labeling_ = labeling;

    this->labeling_valid        = true;
    this->clique_manager_valid  = false;
    this->clique_table_valid    = false;
}

void MgmSolution::set_solution(const CliqueManager &clique_manager) {
    this->cm = clique_manager;
    this->ct = clique_manager.cliques;
    
    this->labeling_valid        = false;
    this->clique_manager_valid  = true;
    this->clique_table_valid    = true;
}

void MgmSolution::set_solution(const CliqueTable &clique_table) {   
    this->ct = clique_table;

    this->labeling_valid        = false;
    this->clique_manager_valid  = false;
    this->clique_table_valid    = true;
}

void MgmSolution::set_solution(Labeling&& labeling) {
    this->labeling_ = std::move(labeling);

    this->labeling_valid        = true;
    this->clique_manager_valid  = false;
    this->clique_table_valid    = false;
}

void MgmSolution::set_solution(CliqueManager&& clique_manager) {
    this->cm = std::move(clique_manager);
    this->ct = this->cm.cliques;
    
    this->labeling_valid        = false;
    this->clique_manager_valid  = true;
    this->clique_table_valid    = true;
}

void MgmSolution::set_solution(CliqueTable&& clique_table) {   
    this->ct = std::move(clique_table);

    this->labeling_valid        = false;
    this->clique_manager_valid  = false;
    this->clique_table_valid    = true;
}

void MgmSolution::set_solution(const GmModelIdx &idx, std::vector<int> labeling)
{
    this->labeling_[idx] = labeling;

    this->clique_manager_valid = false;
    this->clique_table_valid = false;
}

void MgmSolution::set_solution(const GmSolution &sub_solution)
{
    GmModelIdx idx = GmModelIdx(sub_solution.model->graph1.id, sub_solution.model->graph2.id);
    this->set_solution(idx, sub_solution.labeling());
}

Labeling MgmSolution::create_empty_labeling() const
{
    auto res = Labeling();
    res.reserve(this->model->models.size());

    for (const auto& [idx, m] : this->model->models) {
        auto labeling_size = this->model->graphs[idx.first].no_nodes;
        res.emplace(idx, std::vector<int>(labeling_size, -1));
    }

    return res;
}

const std::vector<int> &MgmSolution::operator[](GmModelIdx idx) const {
    return this->labeling().at(idx);
}

double MgmSolution::evaluate() const {
    double result = 0.0;
    for (const auto& [idx, m] : this->model->models) {
        // TODO: Unnecessary vector copying to construct gm solution
        GmSolution gm_sol(m, this->labeling().at(idx));
        result += gm_sol.evaluate();
    }
    return result;
}

double MgmSolution::evaluate(int graph_id) const {
    double result = 0.0;
    for (const auto& [idx, m] : this->model->models) {
        if (idx.first == graph_id || idx.second == graph_id) {
            // TODO: Unnecessary vector copying to construct gm solution
            GmSolution gm_sol(m, this->labeling().at(idx));
            result += gm_sol.evaluate();
        }
    }
    return result;
}

// bool MgmSolution::is_cycle_consistent() const{
//     return true;
// }

mgm::CliqueTable clique_table_from_labeling(mgm::Labeling &labeling, std::vector<mgm::Graph> &graphs) {
    CliqueTable res(graphs.size());

    // 2d array to store clique_idx of every node
    // node_clique_idx[graph_id][node_id] = clique_idx of table
    std::vector<std::vector<int>> node_clique_idx;
    for (const auto& g : graphs) {
        node_clique_idx.emplace_back(g.no_nodes, -1);
    }
    
    for (size_t g1 = 0; g1 < graphs.size(); g1++){
        for (size_t g2 = (g1 + 1); g2 < graphs.size(); g2++) {
            GmModelIdx model_idx(g1,g2);

            // no labeling for model
            if (labeling.find(model_idx) == labeling.end())
                continue;
            
            for (size_t node_id = 0; node_id < labeling[model_idx].size(); node_id++) {
                const int & label = labeling[model_idx][node_id];
                if (label < 0) 
                    continue;
                
                int& clique_idx_n1 = node_clique_idx[g1][node_id];
                int& clique_idx_n2 = node_clique_idx[g2][label];
                if (clique_idx_n1 < 0 && clique_idx_n2 < 0) {
                    CliqueTable::Clique c;
                    c[g1] = node_id;
                    c[g2] = label;
                    res.add_clique(c);
                    clique_idx_n1 = res.no_cliques-1;
                    clique_idx_n2 = res.no_cliques-1;
                }
                else if (clique_idx_n1 >= 0 && clique_idx_n2 < 0) {
                    res[clique_idx_n1][g2] = label;
                    clique_idx_n2 = clique_idx_n1;
                }
                else if (clique_idx_n1 < 0 && clique_idx_n2 >= 0) {
                    res[clique_idx_n2][g1] = node_id;
                    clique_idx_n1 = clique_idx_n2;
                }
                else if (clique_idx_n1 != clique_idx_n2) {
                    throw std::logic_error("Can't transform labeling to set of cliques. Cycle inconsistent labeling in MgmSolution. Nodes matched to each other implied to reside in different cliques because of previously set labeling.");
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
            res.add_clique(c);
        }
    }

    // #ifndef DEBUG
    // // VERIFIES THAT CLIQUES WERE PARSED CORRECTLY
    // // TODO: move to test case.
    // std::vector<int> graph_ids(graphs.size());
    // std::iota(graph_ids.begin(), graph_ids.end(), 0);
    
    // CliqueManager cm(graph_ids, (*this->model));
    // cm.reconstruct_from(res);

    // for (const auto& [model_idx, l] : labeling) {
    //     for (size_t node_id = 0; node_id < l.size(); node_id++) {
    //         const int & label = l[node_id];
    //         if (label < 0)
    //             continue;
    //         assert(cm.clique_idx(model_idx.first, node_id) == cm.clique_idx(model_idx.second, label));
    //     }
    // }
    // #endif

    return res;
}

}