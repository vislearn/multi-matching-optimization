#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <iostream>

#include <omp.h>

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
        
        this->search_order = std::vector<int>(model->no_graphs);
        std::iota(this->search_order.begin(), this->search_order.end(), 0);
    };

    LocalSearcher::LocalSearcher(CliqueManager state, std::vector<int> search_order, std::shared_ptr<MgmModel> model)
        : state(state), search_order(search_order), model(model) {
        auto sol = MgmSolution(model);
        sol.build_from(state.cliques);

        this->current_energy = sol.evaluate();
    };

    bool LocalSearcher::search() {
        this->current_step = 0;
        assert(this->search_order.size() > 0); // Search order was not set
        spdlog::info("Running local search.");

        while (!this->should_stop()) {
            this->current_step++;
            this->previous_energy = this->current_energy;

            spdlog::info("Iteration {}. Current energy: {}", this->current_step, this->current_energy);

            this->iterate();

            spdlog::info("Finished iteration {}\n", this->current_step);
        }

        spdlog::info("Finished local search. Current energy: {}", this->current_energy);
        return (this->last_improved_graph >= 0);
    }

    CliqueManager LocalSearcher::export_CliqueManager() {
        return this->state;
    }

    CliqueTable LocalSearcher::export_cliquetable()
    {
        return this->state.cliques;
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
            if (this->current_step > 1  && graph_id == last_improved_graph) {
                spdlog::info("No improvement since this graph was last checked. Stopping iteration early.");
                return;
            }

            spdlog::info("Resolving for graph {} (step {}/{})", graph_id, idx, this->search_order.size());

            auto managers = details::split(this->state, graph_id, (*this->model));

            GmSolution sol              = details::match(managers.first, managers.second, (*this->model));
            CliqueManager new_manager   = details::merge(managers.first, managers.second, sol, (*this->model));

            // check if improved
            auto mgm_sol = MgmSolution(model);
            mgm_sol.build_from(new_manager.cliques);
            double energy = mgm_sol.evaluate();

            if (energy < this->current_energy) { 
                this->state = new_manager;
                this->current_energy = energy;
                this->last_improved_graph = graph_id;
                spdlog::info("Better solution found. Previous energy: {} ---> Current energy: {}", this->previous_energy, this->current_energy);
            }
            else {
                spdlog::info("Worse solution(Energy: {}) after rematch. Reversing.\n", energy);
            }

            idx++;
        }
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
            if (this->current_step >= this->stopping_criteria.max_steps) {
                spdlog::info("Stopping - Maximum number of iterations reached.\n");
                return true;
            }
        }

        return false;
    }

    LocalSearcherParallel::LocalSearcherParallel(CliqueManager state, std::shared_ptr<MgmModel> model)
    : state(state), model(model) {
            auto sol = MgmSolution(model);
            sol.build_from(state.cliques);

            this->current_energy = sol.evaluate();
            this->matchings.reserve(state.graph_ids.size());
        }

    //FIXME: Is (nearly) same as in LocalSearcher
    bool LocalSearcherParallel::search(){
        this->current_step = 0;
        spdlog::info("Running parallel local search.");
        double initial_energy = this->current_energy;

        while (!this->should_stop()) {
            this->current_step++;
            this->previous_energy = this->current_energy;

            spdlog::info("Iteration {}. Current energy: {}", this->current_step, this->current_energy);

            this->iterate();

            spdlog::info("Finished iteration {}\n", this->current_step);
        }

        spdlog::info("Finished parallel local search. Current energy: {}", this->current_energy);
        return (this->current_energy < initial_energy); //TODO: Make this machine precision safe.
    }

    //FIXME: Is same as in LocalSearcher
    MgmSolution LocalSearcherParallel::export_solution()
    {
        spdlog::info("Exporting solution...");
        MgmSolution sol(this->model);
        sol.build_from(this->state.cliques);
        
        return sol;
    }

    CliqueTable LocalSearcherParallel::export_cliquetable() {
        return this->state.cliques;
    }

    CliqueManager LocalSearcherParallel::export_CliqueManager() {
        return this->state;
    }

    //FIXME: Is same as in LocalSearcher
    bool LocalSearcherParallel::should_stop() {
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
            if (this->current_step >= this->stopping_criteria.max_steps) {
                spdlog::info("Stopping - Maximum number of iterations reached.\n");
                return true;
            }
        }

        return false;
    }

    void LocalSearcherParallel::iterate()
    {   
        spdlog::info("Solving local search for all graphs in parallel...");

        // Disable info logging for the duration of multithreading.
        // Clutters the log otherwise.
        auto log_level = spdlog::get_level();
        spdlog::set_level(spdlog::level::warn);

        // Solve local search for each graph separately.
        #pragma omp parallel
        {
            #pragma omp for
            for (const auto& graph_id : this->state.graph_ids) {
                auto managers = details::split_unpruned(this->state, graph_id, (*this->model));

                GmSolution sol              = details::match(managers.first, managers.second, (*this->model));
                CliqueManager new_manager   = details::merge(managers.first, managers.second, sol, (*this->model));

                auto mgm_sol = MgmSolution(model);
                mgm_sol.build_from(new_manager.cliques);
                double energy = mgm_sol.evaluate();

                #pragma omp critical
                {
                    this->matchings.push_back(std::make_tuple(graph_id, sol, new_manager, energy));
                }
            }
        }

        spdlog::set_level(log_level);
        
        // sort and check for best solution
        static auto lambda_sort_high_energy = [](auto& a, auto& b) { return std::get<3>(a) < std::get<3>(b); };
        std::sort(this->matchings.begin(), this->matchings.end(), lambda_sort_high_energy);
        
        double best_energy      = std::get<3>(this->matchings[0]);
        if (best_energy >= this->current_energy) {
            spdlog::info("No new solution found");
            return;
        }

        // better solution
        this->state             = std::get<2>(this->matchings[0]);

        // readd each graph
        int no_better_solutions = 1;
        int no_graphs_merged = 1;

        for (auto it=this->matchings.begin() + 1 ; it != this->matchings.end(); it++) {
            double& e = std::get<3>(*it);

            // only readd, if energy improved.
            if (e >= this->current_energy) {
                continue;
            }
            no_better_solutions++;

            // Merge into current state.
            auto& graph_id = std::get<0>(*it);
            auto& sol = std::get<1>(*it);

            auto managers               = details::split_unpruned(this->state, graph_id, (*this->model));
            CliqueManager new_manager   = details::merge(managers.first, managers.second, sol, (*this->model));

            // Overwrite solution, if improved.
            auto mgm_sol = MgmSolution(model);
            mgm_sol.build_from(new_manager.cliques);
            double energy = mgm_sol.evaluate();

            if (energy < best_energy) {
                this->state = new_manager;
                best_energy = energy;

                no_graphs_merged++;
                spdlog::info("Improvement found ---> Current energy: {}", best_energy);
            }
        }
        this->state.prune();
        this->current_energy    = best_energy;
        spdlog::info("Better solution found. Previous energy: {} ---> Current energy: {}", this->previous_energy, this->current_energy);
        spdlog::info("Number of better solutions {}. Of which were merged: {}\n", no_better_solutions, no_graphs_merged);
    }

namespace details {
std::pair<CliqueManager, CliqueManager> split_unpruned(const CliqueManager &manager, int graph_id, const MgmModel& model) {
    
    CliqueManager manager_1(manager);
    manager_1.remove_graph(graph_id, false);
    
    CliqueManager manager_2(model.graphs[graph_id]);

    return std::make_pair(manager_1, manager_2);
}

}
}