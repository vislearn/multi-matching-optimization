#ifndef LIBMGM_QAP_INTERFACE_HPP
#define LIBMGM_QAP_INTERFACE_HPP
#include <memory>
#include <vector>
#include <map>

#include <mpopt/qap.h>

#include "multigraph.hpp"
#include "solution.hpp"

namespace mgm {

namespace details {
typedef std::vector<std::vector<double>> DecompCosts;

// Currently a translation from the python interface to c++
// Possibly could be optimized further to allow dynamically adding assignments/edges
class ModelDecomposition {
    public:
        ModelDecomposition(const GmModel& model);

        std::vector<int> no_forward;
        std::vector<int> no_backward;

        std::unordered_map<int, std::unordered_map<int, DecompCosts>> pairwise;

        int gm_id(int qap_node_id);
        int qap_id(int gm_node_id);
        int no_qap_nodes;
    private:
        // Node ID's without any assignments associated with them (yes, may happen),
        // will not be constructed as unaries in the QAP-Solver.
        // These two structures provide the mapping between the id's.
        std::vector<int> qap_node_id_to_model_node_id;
        std::unordered_map<int, int> model_node_id_to_qap_node_id;
        
        void insert_pairwise(const GmModel& model, const EdgeIdx& edge, const double& cost, bool create_new_edges=true);
};
}

class QAPSolver {
    public:
        QAPSolver(std::shared_ptr<GmModel> model, int batch_size=10, int greedy_generations = 10);
        
        GmSolution run(bool verbose=false);

        static inline unsigned long libmpopt_seed = 0;

        struct StoppingCriteria {
            float p = 0.6;
            int k = 5;
            int max_batches = 100;
        };

        StoppingCriteria stopping_criteria;
    private:

        // mpopt_qap_solver is defined in qap.h as a forward declaration.
        //
        // To wrap it into a unique pointer, a custom deleter is needed,
        // Otherwise, unique_ptr tries to create a default deleter (at which it fails).
        struct mpopt_Deleter
        {
            void operator()(mpopt_qap_solver *s);
        };

        details::ModelDecomposition decomposition;
        std::unique_ptr<mpopt_qap_solver, mpopt_Deleter> mpopt_solver;
        std::shared_ptr<GmModel> model;

        int batch_size;
        int greedy_generations;

        void construct_solver();
        GmSolution extract_solution();
        size_t estimate_memory_kib();
};
}
#endif