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
#include "spdlog/spdlog.h"

namespace fs = std::filesystem;

GmSolution::GmSolution(std::shared_ptr<GmModel> model) {
    this->model = model;
    this->labeling = std::vector<int>(model->graph1.no_nodes, -1);
}

MgmSolution::MgmSolution(std::shared_ptr<MgmModel> model) {
    this->model = model;
    gmSolutions.reserve(model->models.size());

    for (auto const& [key, m] : model->models) {
        gmSolutions[key] = GmSolution(m);
    }
}

bool MgmSolution::is_cycle_consistent() {
    return true;
}

void safe_to_disk(const MgmSolution& solution, fs::path outPath) {
    json j;

    j["energy"] = 0.0;

    for (auto const& [key, s] : solution.gmSolutions) {
        std::string key_string = fmt::format("{}, {}", s.model->graph1.id, s.model->graph2.id);
        j["labeling"][key_string] = s.labeling;
    }

    spdlog::info("Saving solution to disk: {}", j.dump());
    std::ofstream o(outPath / "solution.json");
    o << std::setw(4) << j << std::endl;
}