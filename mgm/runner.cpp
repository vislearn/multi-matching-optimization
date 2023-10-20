#include <spdlog/spdlog.h>
#include <libmgm/mgm.hpp>

#include "argparser.hpp"

#include "runner.hpp"

Runner::Runner(ArgParser::Arguments args) : args(args) {
    spdlog::info("Loading model...");

    auto mgmModel = mgm::io::parse_dd_file(args.input_file);
    this->model = std::make_shared<mgm::MgmModel>(std::move(mgmModel));
}

mgm::MgmSolution Runner::run_fast()
{   
    auto solver = mgm::ParallelGenerator(this->model);
    solver.generate();
    
    return solver.export_solution();
}

mgm::MgmSolution Runner::run_balanced()
{
    auto solver = mgm::ParallelGenerator(model);
    solver.generate();
    auto cliques = solver.export_CliqueManager();

    auto local_searcher = mgm::LocalSearcherParallel(cliques, this->model);
    local_searcher.search();

    return local_searcher.export_solution();
}

mgm::MgmSolution Runner::run_incremental()
{
    auto solver = mgm::IncrementalGenerator(5, model);
    solver.init_generation_sequence(mgm::IncrementalGenerator::matching_order::random);
    
    solver.generate();

    return solver.export_solution();
}

mgm::MgmSolution Runner::run_optimal() {
    auto solver = mgm::ParallelGenerator(model);
    solver.generate();
    auto solution_cliquemanager = solver.export_CliqueManager();

    auto local_searcher = mgm::LocalSearcherParallel(solution_cliquemanager, this->model);
    local_searcher.search();

    auto solution_cliquetable = local_searcher.export_cliquetable();
    auto swap_local_searcher = mgm::ABOptimizer(solution_cliquetable, this->model);

    bool improved = true;
    while (improved) {
        improved = swap_local_searcher.search();

        if (improved) {
            solution_cliquetable = swap_local_searcher.export_cliquetable();
            solution_cliquemanager.reconstruct_from(solution_cliquetable);
            local_searcher = mgm::LocalSearcherParallel(solution_cliquemanager, this->model);
            improved = local_searcher.search();
        } else {
            return swap_local_searcher.export_solution();
        }
    }
    return local_searcher.export_solution();
}

mgm::MgmSolution Runner::run() {
    switch (this->args.mode) {

        case ArgParser::optimization_mode::fast:
            return this->run_fast();
            break;

        case ArgParser::optimization_mode::balanced:
            return this->run_balanced();
            break;

        case ArgParser::optimization_mode::incremental:
            return this->run_incremental();
            break;

        case ArgParser::optimization_mode::optimal:
            return this->run_optimal();
            break;

        default:
            throw std::logic_error("Invalid optimization mode. This state should not be reached.");
    }
}