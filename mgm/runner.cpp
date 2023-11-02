#include <stdexcept>
#include <spdlog/spdlog.h>
#include <libmgm/mgm.hpp>

#include "argparser.hpp"

#include "runner.hpp"

Runner::Runner(ArgParser::Arguments args) : args(args) {
    spdlog::info("Loading model...");

    auto mgmModel = mgm::io::parse_dd_file(args.input_file);
    this->model = std::make_shared<mgm::MgmModel>(std::move(mgmModel));
}

mgm::MgmSolution Runner::run_seqseq()
{
    auto solver = mgm::SequentialGenerator(model);
    auto search_order = solver.init_generation_sequence(mgm::SequentialGenerator::matching_order::random);
    solver.generate();

    auto cliques = solver.export_CliqueManager();
    auto local_searcher = mgm::LocalSearcher(cliques, search_order, this->model);
    local_searcher.search();

    return local_searcher.export_solution();
}

mgm::MgmSolution Runner::run_seqpar()
{
    auto solver = mgm::SequentialGenerator(model);
    auto search_order = solver.init_generation_sequence(mgm::SequentialGenerator::matching_order::random);
    solver.generate();

    auto cliques = solver.export_CliqueManager();
    auto local_searcher = mgm::LocalSearcherParallel(cliques, this->model, !this->args.merge_one);
    local_searcher.search();

    return local_searcher.export_solution();
}

mgm::MgmSolution Runner::run_parseq()
{
    auto solver = mgm::ParallelGenerator(model);
    solver.generate();

    auto cliques = solver.export_CliqueManager();
    auto local_searcher = mgm::LocalSearcher(cliques, this->model);
    local_searcher.search();

    return local_searcher.export_solution();
}

mgm::MgmSolution Runner::run_parpar()
{
    auto solver = mgm::ParallelGenerator(model);
    solver.generate();

    auto cliques = solver.export_CliqueManager();
    auto local_searcher = mgm::LocalSearcherParallel(cliques, this->model, !this->args.merge_one);
    local_searcher.search();

    return local_searcher.export_solution();
}

mgm::MgmSolution Runner::run_incseq()
{
    if(this->args.incremental_set_size > this->model->no_graphs)
        throw std::invalid_argument("Incremental set site exceeds number of graphs in the model");
        
    auto solver = mgm::IncrementalGenerator(this->args.incremental_set_size, model);
    auto search_order = solver.init_generation_sequence(mgm::IncrementalGenerator::matching_order::random);
    
    solver.generate();

    auto cliques = solver.export_CliqueManager();
    auto local_searcher = mgm::LocalSearcher(cliques, search_order, this->model);
    local_searcher.search();

    return local_searcher.export_solution();
}

mgm::MgmSolution Runner::run_incpar()
{
    if(this->args.incremental_set_size > this->model->no_graphs)
        throw std::invalid_argument("Incremental set site exceeds number of graphs in the model");
        
    auto solver = mgm::IncrementalGenerator(this->args.incremental_set_size, model);
    (void) solver.init_generation_sequence(mgm::IncrementalGenerator::matching_order::random);
    
    solver.generate();

    auto cliques = solver.export_CliqueManager();
    auto local_searcher = mgm::LocalSearcherParallel(cliques, this->model, !this->args.merge_one);
    local_searcher.search();

    return local_searcher.export_solution();
}

mgm::MgmSolution Runner::run_optimal() {
    auto solver = mgm::SequentialGenerator(model);
    auto search_order = solver.init_generation_sequence(mgm::SequentialGenerator::matching_order::random);
    solver.generate();

    auto cliques = solver.export_CliqueManager();
    auto local_searcher = mgm::LocalSearcher(cliques, search_order, this->model);
    local_searcher.search();

    auto solution_cliquemanager = local_searcher.export_CliqueManager();
    auto solution_cliquetable = local_searcher.export_cliquetable();
    auto swap_local_searcher = mgm::ABOptimizer(solution_cliquetable, this->model);

    bool improved = true;
    while (improved) {
        improved = swap_local_searcher.search();

        if (improved) {
            solution_cliquetable = swap_local_searcher.export_cliquetable();
            solution_cliquemanager.reconstruct_from(solution_cliquetable);
            local_searcher = mgm::LocalSearcher(solution_cliquemanager, this->model);
            improved = local_searcher.search();
        } else {
            return swap_local_searcher.export_solution();
        }
    }
    return local_searcher.export_solution();
}

mgm::MgmSolution Runner::run_optimalpar() {
    auto solver = mgm::ParallelGenerator(model);
    solver.generate();
    
    auto solution_cliquemanager = solver.export_CliqueManager();
    auto local_searcher = mgm::LocalSearcherParallel(solution_cliquemanager, this->model, !this->args.merge_one);
    local_searcher.search();

    auto solution_cliquetable = local_searcher.export_cliquetable();
    auto swap_local_searcher = mgm::ABOptimizer(solution_cliquetable, this->model);

    bool improved = true;
    while (improved) {
        improved = swap_local_searcher.search();

        if (improved) {
            solution_cliquetable = swap_local_searcher.export_cliquetable();
            solution_cliquemanager.reconstruct_from(solution_cliquetable);
            local_searcher = mgm::LocalSearcherParallel(solution_cliquemanager, this->model, !this->args.merge_one);
            improved = local_searcher.search();
        } else {
            return swap_local_searcher.export_solution();
        }
    }
    return local_searcher.export_solution();
}

mgm::MgmSolution Runner::run() {
    switch (this->args.mode) {
        case ArgParser::optimization_mode::seqseq:
            return this->run_seqseq();
            break;
        case ArgParser::optimization_mode::seqpar:
            return this->run_seqpar();
            break;
        case ArgParser::optimization_mode::parseq:
            return this->run_parseq();
            break;
        case ArgParser::optimization_mode::parpar:
            return this->run_parpar();
            break;
        case ArgParser::optimization_mode::incseq:
            return this->run_incseq();
            break;
        case ArgParser::optimization_mode::incpar:
            return this->run_incpar();
            break;
        case ArgParser::optimization_mode::optimal:
            return this->run_optimal();
            break;

        case ArgParser::optimization_mode::optimalpar:
            return this->run_optimalpar();
            break;

        default:
            throw std::logic_error("Invalid optimization mode. This state should not be reached.");
    }
}