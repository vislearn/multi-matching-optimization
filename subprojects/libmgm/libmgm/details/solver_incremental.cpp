#include <cassert>

#include <spdlog/spdlog.h>

#include "solver_local_search.hpp"
#include "solver_incremental.hpp"

namespace mgm {
    IncrementalGenerator::IncrementalGenerator(int subset_size, std::shared_ptr<MgmModel> model)
        : SequentialGenerator(model), subset_size(subset_size) {
            assert(subset_size < model->no_graphs);
        }

    void IncrementalGenerator::generate() {
        // Remove elements from generation queue. Store in a second queue.
        std::queue<CliqueManager> second_queue;
        for (int i = 0; i < this->subset_size-1; i++) {
            second_queue.push(this->generation_queue.front());
            this->generation_queue.pop();
        }
        // !! do not pop last element. Place in the queue needs to be preserved.
        // Will be overwritten later by the intermediate result.
        // Overwrite here with empty CliqueManager just for safety.
        second_queue.push(this->generation_queue.front()); 
        this->generation_queue.front() = CliqueManager(); // Just for safety. Can be omitted.

        std::swap(second_queue, generation_queue);

        spdlog::info("Generating initial solution");
        SequentialGenerator::generate();

        spdlog::info("Improving initial solution");
        this->improve();

        spdlog::info("Solving remaining graphs");

        second_queue.front() = this->current_state; // Write improved result onto the first element in the queue
        this->generation_queue = std::move(second_queue);
        SequentialGenerator::generate();

        MgmSolution sol(this->model);
        sol.build_from(this->current_state.cliques);

        spdlog::info("Fully constructed solution. Current energy: {}", sol.evaluate());
        spdlog::info("Finished incremental generation.\n");
    }

    void IncrementalGenerator::improve() {
        auto search_order = std::vector<int>(this->generation_sequence.begin(), this->generation_sequence.begin() + this->subset_size);
        auto local_searcher = LocalSearcher(this->current_state, search_order, this->model);
        local_searcher.search();

        this->current_state = local_searcher.export_CliqueManager();
    }
}
