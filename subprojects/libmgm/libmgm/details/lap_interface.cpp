#include <cmath> // INFINITY
#include <iostream>
#include <algorithm>
#include <exception>

#include <rectangular_lsap.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ranges.h> // print vector

#include "lap_interface.hpp"
#include "multigraph.hpp"

namespace mgm {



LAPSolver::LAPSolver(std::shared_ptr<GmModel> model)
    : model(model)
{   
    this->nr_rows = model->graph1.no_nodes;
    this->nr_cols = model->graph2.no_nodes + this->nr_rows; // Incomplete LAP transformation: extend with dummy for each row.

    // flat vector representation of cost matrix
    this->costs.resize(this->nr_rows * this->nr_cols, INFINITY);
    
    // Copy assignment costs to flat vector.
    for (auto & a : model->costs->all_assignments()) {
        size_t idx = a.first.first * this->nr_cols + a.first.second;
        this->costs[idx] = a.second;
    }

    // set assignments to dummies to zero
    auto cost_it = this->costs.begin() + model->graph2.no_nodes;
    for (int i = 0; i < this->nr_rows; i++){
        std::fill(cost_it, cost_it+this->nr_rows, 0.0);
        std::advance(cost_it, this->nr_cols);
    }

}

GmSolution LAPSolver::run()
{
    int res = 0;
    std::vector<int64_t> solution_r(this->nr_rows, -1);
    std::vector<int64_t> solution_c(this->nr_cols, -1);

    res = solve_rectangular_linear_sum_assignment(this->nr_rows, this->nr_cols, this->costs.data(), false, solution_r.data(), solution_c.data());

    if (res == RECTANGULAR_LSAP_INFEASIBLE || res == RECTANGULAR_LSAP_INVALID)
        throw std::runtime_error("While solving LAP: LAP  infeasible or invalid");

    GmSolution solution(this->model);

    for (auto & node_id : solution_r) {
        if( solution_c[node_id] < solution.model->graph2.no_nodes)
            solution[node_id] = solution_c[node_id];
    }

    return solution;
}
}