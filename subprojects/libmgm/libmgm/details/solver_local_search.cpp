#include <algorithm>
#include <cassert>
#include <stdexcept>

#include <spdlog/spdlog.h>
#include <fmt/ranges.h> // print vector

#include "solver_mgm.hpp"
#include "random_singleton.hpp"
#include "solution.hpp"

#include "solver_local_search.hpp"
namespace mgm
{

    LocalSearcher::LocalSearcher(CliqueManager state, std::shared_ptr<MgmModel> model)
        : state(state), model(model) {
        auto sol = MgmSolution(model);
        sol.build_from(state.cliques);

        this->current_energy = sol.evaluate();
    };

    void LocalSearcher::search()
    {
        assert(this->search_order.size() > 0); // Search order was not set
        spdlog::info("Running local search.");

        while (!this->should_stop())
        {
            this->current_step++;
            spdlog::info("Iteration {}. Current energy: {}", this->current_step, this->current_energy);
            this->iterate();
            
            
            spdlog::info("Finished iteration {}\n", this->current_step);
            
            set_search_order_random(); // FIXME: write this clean. Function should not depend on search order.
        }
    }

    void LocalSearcher::set_search_order_random()
    {
        this->search_order = this->state.graph_ids;
        RandomSingleton::get().shuffle(this->search_order);
        spdlog::debug("Setting search order: {}", this->search_order);
    }

    MgmSolution LocalSearcher::export_solution() {
        spdlog::info("Exporting solution...");
        MgmSolution sol(this->model);
        sol.build_from(this->state.cliques);
        
        return sol;
    }

    void LocalSearcher::iterate() {
        int idx = 1;
        for (const auto& graph_id : this->search_order) {
            spdlog::debug("Resolving for graph {}/{}", idx, this->search_order.size());

            auto managers = this->split(this->state, graph_id);

            GmSolution sol              = details::match(managers.first, managers.second, (*this->model));
            CliqueManager new_manager   = details::merge(managers.first, managers.second, sol, (*this->model));
            this->state = new_manager;
            idx++;
        }

        auto sol = MgmSolution(model);
        sol.build_from(this->state.cliques);

        this->previous_energy = this->current_energy;
        this->current_energy = sol.evaluate();
    }

    std::pair<CliqueManager, CliqueManager> LocalSearcher::split(const CliqueManager &manager, int graph_id) {
        
        CliqueManager manager_1(this->model->graphs[graph_id]);
        CliqueManager manager_2(manager);

        manager_2.remove_graph(graph_id);

        return std::make_pair(manager_1, manager_2);
    }

    bool LocalSearcher::should_stop() {
        // check stopping criteria
        if (this->stopping_criteria.abstol >= 0)
        {
            if ((previous_energy - current_energy) <= this->stopping_criteria.abstol)
            {
                spdlog::info("Stopping - Absolute increase smaller than defined tolerance.\n");
                return true;
            }
        }
        if (this->stopping_criteria.reltol >= 0)
        {
            throw std::logic_error("Not Implementd");
        }
        if (this->stopping_criteria.max_steps >= 0)
        {
            if (this->current_step > this->stopping_criteria.max_steps) {
                spdlog::info("Stopping - Maximum number of iterations reached.\n");
                return true;
            }
        }

        return false;
    }
}