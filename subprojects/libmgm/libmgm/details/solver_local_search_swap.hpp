#ifndef LIBMGM_SOLVER_LOCAL_SEARCH_SWAP_HPP
#define LIBMGM_SOLVER_LOCAL_SEARCH_SWAP_HPP
#include <vector>
#include <qpbo.h>
#include <unordered_map>
#include <functional>
#include <optional>

#include "cliques.hpp"
#include "multigraph.hpp"
#include "solution.hpp"

namespace mgm {

namespace details {
    using SwapGroup = std::vector<int>;
    std::vector<SwapGroup> build_groups(const std::vector<int>& graphs, const CliqueTable::Clique& A, const CliqueTable::Clique& B, const std::shared_ptr<MgmModel> model);

    class CliqueSwapper {
        public:
            struct Solution {
                bool improved;
                std::vector<int> graphs; // (!) Not all graphs. This stores the subset of graphs contained in at least one of the two cliques involved in each step.
                std::vector<SwapGroup> groups;
                std::vector<int> flip_indices;
                double energy;
            };
            CliqueSwapper(int num_graphs, std::shared_ptr<MgmModel> model, CliqueTable& current_state, int max_iterations_QPBO_I=100);

            bool optimize(CliqueTable::Clique& A, CliqueTable::Clique& B);
            bool optimize_with_empty(CliqueTable::Clique& A);

            bool optimize_no_groups(CliqueTable::Clique& A, CliqueTable::Clique& B);
            bool optimize_with_empty_no_groups(CliqueTable::Clique& A);
            
            CliqueSwapper::Solution current_solution;

        private:
            qpbo::QPBO<double> qpbo_solver;
            std::shared_ptr<MgmModel> model;
            CliqueTable& current_state;

            int max_iterations_QPBO_I = 100;

            bool run_qpbo_solver();
            double star_flip_cost(int id_graph1, int id_graph2, int alpha1, int alpha2, int beta1, int beta2);

    };

    void flip(CliqueTable::Clique& A, CliqueTable::Clique& B, CliqueSwapper::Solution & solution);

}

//TODO: Write "Solver" Superclass, that defines model, current_state and export functions.
class SwapLocalSearcher {
    public:
        SwapLocalSearcher(std::shared_ptr<MgmModel> model);

        int max_iterations = 500;
        int max_iterations_QPBO_I = 100;
        
        //TODO: Return True if state was changed, False if no improvement was found.
        bool search(MgmSolution& input);
        bool search(MgmSolution&& input) = delete; //Prevent search(MgmSolution()) and search(std::move(input))
        
    private:
        int current_step = 0;

        void reset();
        bool iterate();

        void post_iterate_cleanup(std::vector<CliqueTable::Clique> & new_cliques);

        std::shared_ptr<MgmModel>               model;
        CliqueTable                             current_state;
        std::unique_ptr<details::CliqueSwapper>   clique_optimizer;

        // State during iterations
        std::vector<bool> cliques_changed_prev;
        std::vector<bool> cliques_changed;
};

}

#endif