#include <stdexcept>
#include <spdlog/spdlog.h>
#include <libmgm/mgm.hpp>

#include "argparser.hpp"

#include "runner.hpp"

Runner::Runner(ArgParser::Arguments args) : args(args) {
    spdlog::info("Loading model...");
    if (args.unary_constant != 0.0){
        spdlog::info("Using custom unary constant {}", args.unary_constant);
    }
    
    this->model = mgm::io::parse_dd_file(args.input_file, args.unary_constant);

    // If run as a synchronizaiton algorithm, transform the model with the given solution.
    if (args.synchronize || args.synchronize_infeasible) {
        bool feasible = args.synchronize;
        auto s = mgm::io::import_from_disk(this->args.labeling_path, this->model);

        this->model = mgm::build_sync_problem(this->model, s, feasible);
    }
}

mgm::MgmSolution Runner::run_seq() {
    auto solver = mgm::SequentialGenerator(model);
    (void) solver.init(mgm::MgmGenerator::matching_order::random);

    return solver.generate();
}

mgm::MgmSolution Runner::run_par() {
    auto solver = mgm::ParallelGenerator(model);
    (void) solver.init(mgm::MgmGenerator::matching_order::random);
    
    return solver.generate();
}

mgm::MgmSolution Runner::run_inc() {
    if(this->args.incremental_set_size > this->model->no_graphs)
        throw std::invalid_argument("Incremental set size exceeds number of graphs in the model");
        
    auto solver = mgm::IncrementalGenerator(this->args.incremental_set_size, model);
    (void) solver.init(mgm::MgmGenerator::matching_order::random);
    
    return solver.generate();
}

mgm::MgmSolution Runner::run_seqseq()
{
    auto solver = mgm::SequentialGenerator(model);
    auto search_order = solver.init(mgm::MgmGenerator::matching_order::random);
    auto sol = solver.generate();

    auto local_searcher = mgm::GMLocalSearcher(this->model, search_order);
    local_searcher.search(sol);

    return sol;
}

mgm::MgmSolution Runner::run_seqpar()
{
    auto solver = mgm::SequentialGenerator(model);
    auto search_order = solver.init(mgm::MgmGenerator::matching_order::random);
    auto sol = solver.generate();

    auto local_searcher = mgm::GMLocalSearcherParallel(this->model, !this->args.merge_one);
    local_searcher.search(sol);

    return sol;
}

mgm::MgmSolution Runner::run_parseq()
{
    auto solver = mgm::ParallelGenerator(model);
    auto search_order = solver.init(mgm::MgmGenerator::matching_order::random);
    auto sol = solver.generate();

    auto local_searcher = mgm::GMLocalSearcher(this->model, search_order);
    local_searcher.search(sol);

    return sol;
}

mgm::MgmSolution Runner::run_parpar()
{
    auto solver = mgm::ParallelGenerator(model);
    auto search_order = solver.init(mgm::MgmGenerator::matching_order::random);
    auto sol = solver.generate();

    auto local_searcher = mgm::GMLocalSearcherParallel(this->model, !this->args.merge_one);
    local_searcher.search(sol);

    return sol;
}

mgm::MgmSolution Runner::run_incseq()
{
    if(this->args.incremental_set_size > this->model->no_graphs)
        throw std::invalid_argument("Incremental set site exceeds number of graphs in the model");
        
    auto solver = mgm::IncrementalGenerator(this->args.incremental_set_size, model);
    auto search_order = solver.init(mgm::MgmGenerator::matching_order::random);
    
    auto sol = solver.generate();

    auto local_searcher = mgm::GMLocalSearcher(this->model, search_order);
    local_searcher.search(sol);

    return sol;
}

mgm::MgmSolution Runner::run_incpar()
{
    if(this->args.incremental_set_size > this->model->no_graphs)
        throw std::invalid_argument("Incremental set site exceeds number of graphs in the model");
        
    auto solver = mgm::IncrementalGenerator(this->args.incremental_set_size, model);
    (void) solver.init(mgm::MgmGenerator::matching_order::random);
    
    auto sol = solver.generate();

    auto local_searcher = mgm::GMLocalSearcherParallel(this->model, !this->args.merge_one);
    local_searcher.search(sol);

    return sol;
}

mgm::MgmSolution Runner::run_optimal() {
    auto solver = mgm::SequentialGenerator(model);
    auto search_order = solver.init(mgm::MgmGenerator::matching_order::random);
    auto sol = solver.generate();

    auto local_searcher = mgm::GMLocalSearcher(this->model, search_order);
    local_searcher.search(sol);

    auto swap_local_searcher = mgm::SwapLocalSearcher(this->model);

    bool improved = true;
    while (improved) {
        improved = swap_local_searcher.search(sol);

        if (improved) {
            improved = local_searcher.search(sol);
        } else {
            return sol;
        }
    }
    return sol;
}

mgm::MgmSolution Runner::run_optimalpar() {
    auto solver = mgm::ParallelGenerator(model);
    (void) solver.init(mgm::MgmGenerator::matching_order::random);
    auto sol = solver.generate();
    
    auto local_searcher = mgm::GMLocalSearcherParallel(this->model, !this->args.merge_one);
    local_searcher.search(sol);

    auto swap_local_searcher = mgm::SwapLocalSearcher(this->model);

    bool improved = true;
    while (improved) {
        improved = swap_local_searcher.search(sol);

        if (improved) {
            local_searcher = mgm::GMLocalSearcherParallel(this->model, !this->args.merge_one);

            improved = local_searcher.search(sol);
        } else {
            return sol;
        }
    }
    return sol;
}

mgm::MgmSolution Runner::run_improve_swap()
{
    auto sol = mgm::io::import_from_disk(this->args.labeling_path, this->model);

    auto swap_local_searcher = mgm::SwapLocalSearcher(this->model);
    swap_local_searcher.search(sol);

    return sol;
}

mgm::MgmSolution Runner::run_improve_qap()
{
    auto sol = mgm::io::import_from_disk(this->args.labeling_path, this->model);

    std::vector<int> graph_ids(this->model->no_graphs);
    std::iota(graph_ids.begin(), graph_ids.end(), 0);

    auto local_searcher = mgm::GMLocalSearcher(this->model);
    local_searcher.search(sol);

    return sol;
}

mgm::MgmSolution Runner::run_improve_qap_par()
{
    auto sol = mgm::io::import_from_disk(this->args.labeling_path, this->model);

    std::vector<int> graph_ids(this->model->no_graphs);
    std::iota(graph_ids.begin(), graph_ids.end(), 0);

    auto local_searcher = mgm::GMLocalSearcherParallel(this->model, !this->args.merge_one);
    local_searcher.search(sol);

    return sol;
}

mgm::MgmSolution Runner::run_improveopt()
{
    auto sol = mgm::io::import_from_disk(this->args.labeling_path, this->model);

    std::vector<int> graph_ids(this->model->no_graphs);
    std::iota(graph_ids.begin(), graph_ids.end(), 0);

    auto local_searcher = mgm::GMLocalSearcher(this->model);
    local_searcher.search(sol);

    auto swap_local_searcher = mgm::SwapLocalSearcher(this->model);

    bool improved = true;
    while (improved) {
        improved = swap_local_searcher.search(sol);

        if (improved) {
            improved = local_searcher.search(sol);
        } else {
            return sol;
        }
    }
    return sol;
}

mgm::MgmSolution Runner::run_improveopt_par()
{
    auto sol = mgm::io::import_from_disk(this->args.labeling_path, this->model);

    std::vector<int> graph_ids(this->model->no_graphs);
    std::iota(graph_ids.begin(), graph_ids.end(), 0);

    auto local_searcher = mgm::GMLocalSearcherParallel(this->model);
    local_searcher.search(sol);

    auto swap_local_searcher = mgm::SwapLocalSearcher(this->model);

    bool improved = true;
    while (improved) {
        improved = swap_local_searcher.search(sol);

        if (improved) {
            improved = local_searcher.search(sol);
        } else {
            return sol;
        }
    }
    return sol;
}

mgm::MgmSolution Runner::run() {
    switch (this->args.mode) {
        case ArgParser::optimization_mode::seq:
            return this->run_seq();
            break;
        case ArgParser::optimization_mode::par:
            return this->run_par();
            break;
        case ArgParser::optimization_mode::inc:
            return this->run_inc();
            break;
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
        case ArgParser::optimization_mode::improve_swap:
            return this->run_improve_swap();
            break;        
        case ArgParser::optimization_mode::improve_qap:
            return this->run_improve_qap();
            break;        
        case ArgParser::optimization_mode::improve_qap_par:
            return this->run_improve_qap_par();
            break;        
        case ArgParser::optimization_mode::improveopt:
            return this->run_improveopt();
            break;
        case ArgParser::optimization_mode::improveopt_par:
            return this->run_improveopt_par();
            break;

        default:
            throw std::logic_error("Invalid optimization mode. This state should not be reached.");
    }
}