#include <map>
#include <vector>
#include <memory>
#include <utility>
#include <filesystem>
#include <fstream>
#include <fmt/core.h>

// json
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "solution.hpp"
#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

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

void safe_to_disk(const MgmSolution& solution, fs::path outPath) {
    json j;

    j["energy"] = solution.evaluate();

    for (auto const& [key, s] : solution.gmSolutions) {
        std::string key_string = fmt::format("{}, {}", s.model->graph1.id, s.model->graph2.id);
        j["labeling"][key_string] = s.labeling;
    }

    spdlog::debug("Saving solution to disk: {}", j.dump());
    std::ofstream o(outPath / "solution.json");
    o << std::setw(4) << j << std::endl;
}