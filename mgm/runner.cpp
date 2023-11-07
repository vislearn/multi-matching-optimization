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

mgm::MgmSolution Runner::run_seq() {
    auto solver = mgm::SequentialGenerator(model);
    auto search_order = solver.init_generation_sequence(mgm::SequentialGenerator::matching_order::random);
    solver.generate();

    return solver.export_solution();
}

mgm::MgmSolution Runner::run_par() {
    auto solver = mgm::ParallelGenerator(model);
    solver.generate();

    return solver.export_solution();
}

mgm::MgmSolution Runner::run_inc() {
    if(this->args.incremental_set_size > this->model->no_graphs)
        throw std::invalid_argument("Incremental set site exceeds number of graphs in the model");
        
    auto solver = mgm::IncrementalGenerator(this->args.incremental_set_size, model);
    (void) solver.init_generation_sequence(mgm::IncrementalGenerator::matching_order::random);
    
    solver.generate();

    return solver.export_solution();
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

    auto cliquemanager = local_searcher.export_CliqueManager();
    auto cliquetable = local_searcher.export_cliquetable();
    auto swap_local_searcher = mgm::ABOptimizer(cliquetable, this->model);

    bool improved = true;
    while (improved) {
        improved = swap_local_searcher.search();

        if (improved) {
            cliquetable = swap_local_searcher.export_cliquetable();
            cliquemanager.reconstruct_from(cliquetable);
            local_searcher = mgm::LocalSearcher(cliquemanager, this->model);
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
    
    auto cliquemanager = solver.export_CliqueManager();
    auto local_searcher = mgm::LocalSearcherParallel(cliquemanager, this->model, !this->args.merge_one);
    local_searcher.search();

    auto cliquetable = local_searcher.export_cliquetable();
    auto swap_local_searcher = mgm::ABOptimizer(cliquetable, this->model);

    bool improved = true;
    while (improved) {
        improved = swap_local_searcher.search();

        if (improved) {
            cliquetable = swap_local_searcher.export_cliquetable();
            cliquemanager.reconstruct_from(cliquetable);
            local_searcher = mgm::LocalSearcherParallel(cliquemanager, this->model, !this->args.merge_one);
            improved = local_searcher.search();
        } else {
            return swap_local_searcher.export_solution();
        }
    }
    return local_searcher.export_solution();
}


mgm::MgmSolution Runner::run_improve_swap()
{
    auto s = mgm::io::import_from_disk(this->model, this->args.labeling_path);
    auto cliquetable = s.export_cliquetable();

    auto swap_local_searcher = mgm::ABOptimizer(cliquetable, this->model);
    swap_local_searcher.search();

    return swap_local_searcher.export_solution();
}

mgm::MgmSolution Runner::run_improve_qap()
{
    auto s = mgm::io::import_from_disk(this->model, this->args.labeling_path);
    auto cliquetable = s.export_cliquetable();

    std::vector<int> graph_ids(this->model->no_graphs);
    std::iota(graph_ids.begin(), graph_ids.end(), 0);
    
    mgm::CliqueManager cm(graph_ids, (*this->model));
    cm.reconstruct_from(cliquetable);

    auto local_searcher = mgm::LocalSearcher(cm, this->model);
    local_searcher.search();

    return local_searcher.export_solution();
}

mgm::MgmSolution Runner::run_improve_qap_par()
{
    auto s = mgm::io::import_from_disk(this->model, this->args.labeling_path);
    auto cliquetable = s.export_cliquetable();

    std::vector<int> graph_ids(this->model->no_graphs);
    std::iota(graph_ids.begin(), graph_ids.end(), 0);
    
    mgm::CliqueManager cm(graph_ids, (*this->model));
    cm.reconstruct_from(cliquetable);

    auto local_searcher = mgm::LocalSearcherParallel(cm, this->model, !this->args.merge_one);
    local_searcher.search();

    return local_searcher.export_solution();
}

mgm::MgmSolution Runner::run_improveopt()
{
    auto s = mgm::io::import_from_disk(this->model, this->args.labeling_path);
    auto cliquetable = s.export_cliquetable();

    std::vector<int> graph_ids(this->model->no_graphs);
    std::iota(graph_ids.begin(), graph_ids.end(), 0);
    
    mgm::CliqueManager cliquemanager(graph_ids, (*this->model));
    cliquemanager.reconstruct_from(cliquetable);

    auto local_searcher = mgm::LocalSearcher(cliquemanager, this->model);
    local_searcher.search();

    cliquetable = local_searcher.export_cliquetable();
    auto swap_local_searcher = mgm::ABOptimizer(cliquetable, this->model);

    bool improved = true;
    while (improved) {
        improved = swap_local_searcher.search();

        if (improved) {
            cliquetable = swap_local_searcher.export_cliquetable();
            cliquemanager.reconstruct_from(cliquetable);
            local_searcher = mgm::LocalSearcher(cliquemanager, this->model);
            improved = local_searcher.search();
        } else {
            return swap_local_searcher.export_solution();
        }
    }
    return local_searcher.export_solution();
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

        default:
            throw std::logic_error("Invalid optimization mode. This state should not be reached.");
    }
}