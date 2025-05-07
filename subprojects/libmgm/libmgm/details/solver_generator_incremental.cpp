#include <cassert>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ranges.h> // print vector

#include "solver_local_search_GM.hpp"
#include "solver_generator_incremental.hpp"

namespace mgm {
    IncrementalGenerator::IncrementalGenerator(int subset_size, std::shared_ptr<MgmModel> model)
        : SequentialGenerator(model), subset_size(subset_size) {
            assert(subset_size < model->no_graphs);
        }

    MgmSolution IncrementalGenerator::generate() {
        // Remove elements from generation queue. Store in a second queue.
        std::queue<CliqueManager> second_queue;
        for (int i = 0; i < this->subset_size-1; i++) { // -1: this->current_state already contains one object.
            second_queue.push(this->generation_queue.front());
            this->generation_queue.pop();
        }

        std::swap(second_queue, generation_queue);

        spdlog::info("Generating initial solution");
        (void) SequentialGenerator::generate();

        spdlog::info("Improving initial solution");
        this->improve();

        spdlog::info("Solving remaining graphs");
        this->generation_queue = std::move(second_queue);
        (void) SequentialGenerator::generate();

        spdlog::info("Fully constructed solution. Current energy: {}", this->current_state.evaluate());
        spdlog::info("Finished incremental generation.\n");

        return this->current_state;
    }

    void IncrementalGenerator::improve() {
        auto search_order = std::vector<int>(this->generation_sequence.begin(), this->generation_sequence.begin() + this->subset_size);
        spdlog::info("Search order: {}", search_order);
        
        auto local_searcher = GMLocalSearcher(this->model, search_order);
        (void) local_searcher.search(this->current_state);
    }
}
