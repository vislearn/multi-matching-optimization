#include <functional>
#include <optional>

#include "solver_mgm.hpp"
#include "multigraph.hpp"

namespace mgm {

constexpr double INFINITY_COST = 1e99;

//FIXME: This needs a better name.
class LocalSearcher {
    public:
        struct StoppingCriteria {
            int max_steps = 10000;
            double abstol = 0.0;
            double reltol = -1.0;
        };
        
        LocalSearcher(std::shared_ptr<MgmModel> model);
        LocalSearcher(std::shared_ptr<MgmModel> model, std::vector<int> search_order);

        StoppingCriteria stopping_criteria;
        bool search(MgmSolution& input);
        bool search(MgmSolution&& input) = delete; //Prevent search(MgmSolution()) and search(std::move(input))

    private:
        int current_step = 0;
        double previous_energy = INFINITY_COST;
        double current_energy = 0.0;

        void iterate();
        std::optional<std::reference_wrapper<MgmSolution>> current_state;

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
        
        LocalSearcherParallel(std::shared_ptr<MgmModel> model, bool merge_all=true);

        StoppingCriteria stopping_criteria;
        bool search(MgmSolution& input);
        bool search(MgmSolution&& input) = delete; //Prevent search(MgmSolution()) and search(std::move(input))

    private:
        int current_step = 0;
        double previous_energy = INFINITY_COST;
        double current_energy = 0.0;

        void iterate();
        std::optional<std::reference_wrapper<MgmSolution>> current_state;

        using GraphID = int;
        std::vector<std::tuple<GraphID, GmSolution, CliqueManager, double>> matchings;

        std::shared_ptr<MgmModel> model;
        bool merge_all;

        bool should_stop();
};

namespace details {
    // Splits off graph [graph_id] from manager
    // Does not remove any potential empty cliques, to ensure their order and index remain valid. (See Parallel Local Searcher)
    std::pair<CliqueManager, CliqueManager> split_unpruned(const CliqueManager& manager, int graph_id, const MgmModel& model);
}
}