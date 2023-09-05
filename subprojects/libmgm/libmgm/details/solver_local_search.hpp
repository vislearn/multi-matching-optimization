#include "solver_mgm.hpp"
#include "multigraph.hpp"

namespace mgm {

constexpr double INFINTIY_COST = 1e99;

//FIXME: This needs a better name.
class LocalSearcher {
    public:
        struct StoppingCriteria {
            int max_steps = 10000;
            double abstol = 0.0;
            double reltol = -1.0;
        };
        
        LocalSearcher(CliqueManager state, std::shared_ptr<MgmModel> model);

        StoppingCriteria stopping_criteria;
        void search();
        void set_search_order_random();
        
        MgmSolution export_solution();

    private:
        int current_step = 0;
        double previous_energy = INFINTIY_COST;
        double current_energy = 0.0;

        void iterate();
        std::vector<int> search_order;

        CliqueManager state;
        std::shared_ptr<MgmModel> model;

        std::pair<CliqueManager, CliqueManager> split(const CliqueManager& manager, int graph_id);
        bool should_stop();
};

}