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
        
        LocalSearcher(CliqueManager state, std::vector<int> search_order, std::shared_ptr<MgmModel> model);

        StoppingCriteria stopping_criteria;
        void search();
        
        CliqueManager export_CliqueManager();
        MgmSolution export_solution();

    private:
        int current_step = 0;
        double previous_energy = INFINTIY_COST;
        double current_energy = 0.0;

        void iterate();
        CliqueManager state;
        std::vector<int> search_order;
        std::shared_ptr<MgmModel> model;

        int last_improved_graph = -1;
        bool should_stop();
};

//FIXME: This needs a better name.
class LocalSearcherParallel {
    public:
        struct StoppingCriteria {
            int max_steps = 10000;
            double abstol = 0.0;
            double reltol = -1.0;
        };
        
        LocalSearcherParallel(CliqueManager state, std::shared_ptr<MgmModel> model);

        StoppingCriteria stopping_criteria;
        bool search();
        
        MgmSolution export_solution();
        CliqueTable export_cliquetable();

    private:
        int current_step = 0;
        double previous_energy = INFINTIY_COST;
        double current_energy = 0.0;

        void iterate();
        CliqueManager state;

        using GraphID = int;
        std::vector<std::tuple<GraphID, GmSolution, CliqueManager, double>> matchings;

        std::shared_ptr<MgmModel> model;

        bool should_stop();
};

namespace details {
    // Splits off graph [graph_id] from manager
    // Does not remove any potential empty cliques, to ensure their order and index remain valid. (See Parallel Local Searcher)
    std::pair<CliqueManager, CliqueManager> split_unpruned(const CliqueManager& manager, int graph_id, const MgmModel& model);
}
}