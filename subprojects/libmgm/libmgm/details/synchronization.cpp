#include <iostream>

#include <spdlog/spdlog.h>

#include "solution.hpp"
#include "multigraph.hpp"
#include "synchronization.hpp"

namespace mgm {
std::shared_ptr<MgmModel> build_sync_problem(std::shared_ptr<MgmModel> model, MgmSolution &solution, bool feasible) {
    spdlog::info("Building synchronization problem from given model and solution.");

    auto sync_model = std::make_shared<MgmModel>();

    sync_model->no_graphs = model->no_graphs;
    sync_model->graphs = model->graphs;

    int i = 0;
    for (const auto& [key, gm_model] : model->models) {

        // Progress, prints iterations on terminal.
        i++;
        std::cout << i << "/" << model->models.size() << " \r";
        std::cout.flush();

        std::shared_ptr<GmModel> sync_gm_model;

        if (feasible) {
            // only allow assignments that are present
            sync_gm_model = details::create_feasible_sync_model(gm_model, solution.gmSolutions[key]);
        }
        else {
            // allow all assignments
            sync_gm_model = details::create_infeasible_sync_model(gm_model, solution.gmSolutions[key]);
        }
        sync_model->models[key] = sync_gm_model;
    }

    return sync_model;
}


namespace details {
std::shared_ptr<GmModel>  create_feasible_sync_model(std::shared_ptr<GmModel> model, GmSolution& solution) {
    auto sync_model = std::make_shared<GmModel>(model->graph1, model->graph2, model->no_assignments, 0);
    
    // Copy assignments
    // TODO: This is unnecessarily inefficient. Cost datastructure might be too restricted.
    size_t i = 0;
    for (const auto& idx : model->assignment_list) {
        sync_model->add_assignment(i, idx.first, idx.second, 0);
        i++;
    }
    
    // set labeled assignments
    for (size_t i = 0; i < solution.labeling.size(); ++i) {
        if (solution.labeling[i] == -1)
            continue;
        
        sync_model->costs->set_unary(i, solution.labeling[i], -1);
    }

    return sync_model;
}

std::shared_ptr<GmModel>  create_infeasible_sync_model(std::shared_ptr<GmModel> model, GmSolution& solution) {
    int no_assignments = model->graph1.no_nodes * model->graph2.no_nodes;
    auto sync_model = std::make_shared<GmModel>(model->graph1, model->graph2, no_assignments, 0);

    // Initialize all costs to 0
    int idx = 0;
    for (auto i = 0; i  < model->graph1.no_nodes; i++) {
        for (auto j = 0; j  < model->graph2.no_nodes; j++) {
            sync_model->add_assignment(idx, i, j, 0);
            idx++;
        }
    }

    // set labeled assignments
    for (size_t i = 0; i < solution.labeling.size(); ++i) {
        if (solution.labeling[i] == -1)
            continue;
        
        sync_model->costs->set_unary(i, solution.labeling[i], -1);
    }

    return sync_model;
};

}}