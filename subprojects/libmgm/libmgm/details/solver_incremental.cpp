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
        // Construct new generation queue for subset_size
        std::queue<CliqueManager> second_queue;
        for (int i = 0; i < this->subset_size; i++) {
            second_queue.push(this->generation_queue.front());
            this->generation_queue.pop();
        }
        std::swap(second_queue, generation_queue);

        spdlog::info("Generating initial solution");
        SequentialGenerator::generate();
        
        spdlog::info("Improving initial solution");
        this->improve();

        spdlog::info("Solving remaining graphs");
        generation_queue = std::move(second_queue);
        generation_queue.push(this->current_state);
        SequentialGenerator::generate();
    }

    void IncrementalGenerator::improve() {
        auto search_order = std::vector<int>(this->generation_sequence.begin(), this->generation_sequence.begin() + this->subset_size);
        auto local_searcher = LocalSearcher(this->current_state, search_order, this->model);
        local_searcher.search();

        this->current_state = local_searcher.export_CliqueManager();
    }
}
