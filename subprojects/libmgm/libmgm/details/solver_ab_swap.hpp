#ifndef LIBMGM_SOLVER_AB_SWAP_HPP
#define LIBMGM_SOLVER_AB_SWAP_HPP
#include <vector>
#include <qpbo.h>

#include "cliques.hpp"
#include "multigraph.hpp"
#include "solution.hpp"

namespace mgm {

namespace details {
    class CliqueSwapper {
        public:
            struct Solution {
                bool improved;
                std::vector<int> graphs;
                std::vector<int> graph_flip_indices;
                double energy;
            };
            CliqueSwapper(int num_graphs, std::shared_ptr<MgmModel> model, CliqueTable& current_state, int max_iterations_QPBO_I=100);

            bool optimize(CliqueTable::Clique& A, CliqueTable::Clique& B);
            bool optimize_with_empty(CliqueTable::Clique& A);
            
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
class ABOptimizer {
    public:
        ABOptimizer(CliqueTable state, std::shared_ptr<MgmModel> model);

        int max_iterations = 500;
        int max_iterations_QPBO_I = 100;
        
        //TODO: Return True if state was changed, False if no improvement was found.
        bool search();

        CliqueTable export_cliquetable();
        MgmSolution export_solution();
        
    private:
        int current_step = 0;

        void reset();
        bool iterate();

        void post_iterate_cleanup(std::vector<CliqueTable::Clique> & new_cliques);

        CliqueTable current_state;
        std::shared_ptr<MgmModel> model;
        details::CliqueSwapper clique_optimizer;

        // State during iterations
        // TODO: Maybe write custom CliqueManager for ab_swap to keep track of these properties. They belong neither truly to CliqueTable nor to this class.
        std::vector<bool> cliques_changed_prev;
        std::vector<bool> cliques_changed;
};

}

#endif