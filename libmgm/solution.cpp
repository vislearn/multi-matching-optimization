#include <map>
#include <vector>
#include <memory>
#include <utility>

#include "solution.hpp"
#include "spdlog/spdlog.h"

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
};