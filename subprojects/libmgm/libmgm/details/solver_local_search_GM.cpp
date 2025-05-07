#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <iostream>
#include <numeric>

#include <omp.h>

#include <spdlog/spdlog.h>

#include "solver_generator_mgm.hpp"
#include "random_singleton.hpp"
#include "solution.hpp"

#include "solver_local_search_GM.hpp"
namespace mgm
{
    GMLocalSearcher::GMLocalSearcher(std::shared_ptr<MgmModel> model) : model(model) {
        this->search_order = std::vector<int>(model->no_graphs);
        std::iota(this->search_order.begin(), this->search_order.end(), 0);
    }

    GMLocalSearcher::GMLocalSearcher(std::shared_ptr<MgmModel> model, std::vector<int> search_order)
        : search_order(search_order), model(model) {}

    bool GMLocalSearcher::search(MgmSolution& input) {
        assert(this->search_order.size() > 0); // Search order was not set

        this->current_state = input;
        this->current_energy = input.evaluate();
        this->current_step = 0;

        spdlog::info("Running local search.");
        while (!this->should_stop()) {
            this->current_step++;
            this->previous_energy = this->current_energy;

            spdlog::info("Iteration {}. Number of cliques: {}. Current energy: {}.", 
                                                                    this->current_step, 
                                                                    this->current_state->get().clique_manager().cliques.no_cliques, 
                                                                    this->current_energy);
            this->iterate();

            spdlog::info("Finished iteration {}\n", this->current_step);
        }

        spdlog::info("Finished local search. Current energy: {}", this->current_energy);
        return (this->last_improved_graph >= 0);
    }

    void GMLocalSearcher::iterate() {
        int idx = 1;

        for (const auto& graph_id : this->search_order) {
            if (this->current_step > 1  && graph_id == last_improved_graph) {
                spdlog::info("No improvement since this graph was last checked. Stopping iteration early.");
                return;
            }

            spdlog::info("Resolving for graph {} (step {}/{})", graph_id, idx, this->search_order.size());

            auto managers = details::split(this->current_state->get().clique_manager(), graph_id, (*this->model));

            GmSolution sol              = details::match(managers.first, managers.second, (*this->model));
            CliqueManager new_manager   = details::merge(managers.first, managers.second, sol, (*this->model));

            // check if improved
            auto graph_energy_prev = this->current_state->get().evaluate(graph_id);
            spdlog::info("graph_energy_prev: {}", graph_energy_prev);
            
            auto mgm_sol = MgmSolution(model);
            mgm_sol.set_solution(new_manager);
            auto graph_energy_new = mgm_sol.evaluate(graph_id);
            
            spdlog::info("graph_energy_new: {}", graph_energy_new);

            if (graph_energy_new < graph_energy_prev) { 
                this->current_state->get().set_solution(std::move(new_manager));
                this->current_energy += (graph_energy_new - graph_energy_prev);
                this->last_improved_graph = graph_id;
                spdlog::info("Better solution found. Previous energy: {} ---> Current energy: {}", this->previous_energy, this->current_energy);
            }
            else {
                spdlog::info("Worse solution(Energy: {}) after rematch. Reversing.\n", this->current_energy + (graph_energy_new - graph_energy_prev));
            }

            idx++;
        }
    }

    bool GMLocalSearcher::should_stop() {
        // check stopping criteria
        if (this->stopping_criteria.abstol >= 0 && !(previous_energy >= INFINITY_COST || current_energy >= INFINITY_COST))
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

    GMLocalSearcherParallel::GMLocalSearcherParallel(std::shared_ptr<MgmModel> model, bool merge_all)
        : model(model), merge_all(merge_all) {}

    //FIXME: Is (nearly) same as in GMLocalSearcher
    bool GMLocalSearcherParallel::search(MgmSolution& input){
        this->current_state = input;
        this->current_energy = input.evaluate();
        this->previous_energy = INFINITY_COST;
        this->current_step = 0;

        this->matchings.reserve(input.clique_manager().graph_ids.size());

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

    //FIXME: Is same as in GMLocalSearcher
    bool GMLocalSearcherParallel::should_stop() {
        // check stopping criteria
        if (this->stopping_criteria.abstol >= 0 && !(previous_energy >= INFINITY_COST || current_energy >= INFINITY_COST))
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

    void GMLocalSearcherParallel::iterate()
    {   
        spdlog::info("Solving local search for all graphs in parallel...");
        const auto& curr_manager = this->current_state->get().clique_manager();

        // Disable info logging for the duration of multithreading.
        // Clutters the log otherwise.
        auto log_level = spdlog::get_level();
        spdlog::set_level(spdlog::level::warn);

        // Solve local search for each graph separately.
        #pragma omp parallel
        {
            #pragma omp for
            for (size_t i = 0; i < curr_manager.graph_ids.size(); ++i) {
                const auto& graph_id = curr_manager.graph_ids[i];

                auto managers = details::split_unpruned(curr_manager, graph_id, (*this->model));

                GmSolution sol              = details::match(managers.first, managers.second, (*this->model));
                CliqueManager new_manager   = details::merge(managers.first, managers.second, sol, (*this->model));
                
                auto graph_energy_prev = this->current_state->get().evaluate(graph_id);
           
                auto mgm_sol = MgmSolution(model);
                mgm_sol.set_solution(new_manager);
                auto graph_energy_new = mgm_sol.evaluate(graph_id);

                double energy = this->current_energy + (graph_energy_new - graph_energy_prev);

                #pragma omp critical
                {
                    this->matchings.push_back(std::make_tuple(graph_id, std::move(sol), std::move(new_manager), energy));
                }
            }
        }

        spdlog::set_level(log_level);
        
        // sort and check for best solution
        static auto lambda_sort_energy_asc = [](auto& a, auto& b) { return std::get<3>(a) < std::get<3>(b); };
        std::sort(this->matchings.begin(), this->matchings.end(), lambda_sort_energy_asc);
        
        double best_energy = std::get<3>(this->matchings[0]);
        if (best_energy >= this->current_energy) {
            spdlog::info("No new solution found");
            return;
        }

        // better solution
        this->current_state->get().set_solution(std::move(std::get<2>(this->matchings[0])));

        // readd each graph
        int no_better_solutions = 1;
        int no_graphs_merged = 1;
        
        if (this->merge_all) {
            // TODO: Move to extra function
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

                auto managers       = details::split_unpruned(this->current_state->get().clique_manager(), graph_id, (*this->model));
                auto new_manager    = details::merge(managers.first, managers.second, sol, (*this->model));

                // Overwrite solution, if improved.               
                auto graph_energy_prev = this->current_state->get().evaluate(graph_id);
           
                auto mgm_sol = MgmSolution(model);
                mgm_sol.set_solution(new_manager);
                auto graph_energy_new = mgm_sol.evaluate(graph_id);

                double energy = this->current_energy + (graph_energy_new - graph_energy_prev);

                if (energy < best_energy) {
                    this->current_state->get().set_solution(std::move(new_manager));
                    best_energy = energy;

                    no_graphs_merged++;
                    spdlog::info("Improvement found ---> Current energy: {}", best_energy);
                }
            }
        }
        CliqueManager final_solution = this->current_state->get().clique_manager();
        final_solution.prune();
        this->current_state->get().set_solution(std::move(final_solution));

        this->current_energy    = best_energy;
        spdlog::info("Better solution found. Previous energy: {} ---> Current energy: {}", this->previous_energy, this->current_energy);
        spdlog::info("Number of better solutions {}. Of which were merged: {}\n", no_better_solutions, no_graphs_merged);
    }

namespace details {
std::pair<CliqueManager, CliqueManager> split_unpruned(const CliqueManager &manager, int graph_id, const MgmModel& model) {
    
    CliqueManager manager_1(manager);
    manager_1.remove_graph(graph_id, false);
    
    // TODO: This unnecessarily requires graphs be sorted and assending from 0.
    CliqueManager manager_2(model.graphs[graph_id]);

    return std::make_pair(manager_1, manager_2);
}

}
}