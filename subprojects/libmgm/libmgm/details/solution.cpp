#include <map>
#include <vector>
#include <memory>
#include <utility>
#include <filesystem>
#include <fstream>

#include "multigraph.hpp"
#include "solution.hpp"
#include "cliques.hpp"

namespace mgm {

constexpr double INFINTIY_COST = 1e99;

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
                return INFINTIY_COST;
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