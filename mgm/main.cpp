#include <iostream>
#include <unistd.h>
#include <ios>
#include <fstream>
#include <string>
#include <memory>

#include <chrono>

// Logging
#include <spdlog/spdlog.h>
#include <fmt/ranges.h> // print vector

// json
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <qpbo.h>
#pragma clang diagnostic pop

#include <libmgm/mgm.hpp>
#include <omp.h>

#include "argparser.hpp"
#include "runner.hpp"

using namespace std;
void print_mem_usage() {
    double vm_usage = 0.0;
    double resident_set = 0.0;
    ifstream stat_stream("/proc/self/stat",ios_base::in); //get info from proc directory
    //create some variables to get info
    string pid, comm, state, ppid, pgrp, session, tty_nr;
    string tpgid, flags, minflt, cminflt, majflt, cmajflt;
    string utime, stime, cutime, cstime, priority, nice;
    string O, itrealvalue, starttime;
    unsigned long vsize;
    long rss;
    stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
    >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
    >> utime >> stime >> cutime >> cstime >> priority >> nice
    >> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest
    stat_stream.close();
    long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // for x86-64 is configured to use 2MB pages
    vm_usage = vsize / 1024.0;
    resident_set = rss * page_size_kb;

    spdlog::info("++++++++++++++++++++++++++++");
    spdlog::info("--After Parsing input file--");
    spdlog::info("| Virtual Memory: {}MB", vm_usage / 1024.0);
    spdlog::info("| Resident set size: {}MB",resident_set / 1024.0);
    spdlog::info("----------------------------");
}

// void testing(int argc, char **argv) {
//     ArgParser parser(argc, argv);
//     mgm::init_logger(parser.output_path);

//     spdlog::info("----PARSER TEST----");
//     auto mgmModel = mgm::io::parse_dd_file(parser.input_file);
    
//     print_mem_usage();
    
//     double u = mgmModel.models[mgm::GmModelIdx(0,1)]->costs->unary(0,0);
//     spdlog::info("Model (0,1) contains cost (0,0) = {}", u);

//     spdlog::info("----MODEL TEST----");
//     auto model = std::make_shared<mgm::MgmModel>(std::move(mgmModel));
    
//     auto sol = mgm::MgmSolution(model);

//     spdlog::info("----SOLUTION TEST----");
//     for (auto & [key, gm_sol] : sol.gmSolutions) {
//         int no_left = gm_sol.model->graph1.no_nodes;
//         int no_right = gm_sol.model->graph2.no_nodes;
//         for (auto i = 0; i < no_left; i++) {
//             gm_sol.labeling[i] = i;
//             if (i >= no_right) {
//                 break;
//             }
//         }

//         std::ostringstream oss;

//         // Convert all but the last element to avoid a trailing ","
//         std::copy(gm_sol.labeling.begin(), gm_sol.labeling.end()-1,
//             std::ostream_iterator<int>(oss, ","));

//         // Now add the last element with no delimiter
//         oss << gm_sol.labeling.back();
//         spdlog::info("G{}: {} nodes", gm_sol.model->graph1.id, gm_sol.model->graph1.no_nodes);
//         spdlog::info("G{}: {} nodes", gm_sol.model->graph2.id, gm_sol.model->graph2.no_nodes);
        
//         spdlog::info("L{}-{}: {}", key.first, key.second, oss.str());
//     }
//     spdlog::info("----JSON TEST----");
//     json ex1 = json::parse(R"(
//     {
//         "pi": 3.141,
//         "happy": true
//     }
//     )");
    
//     double pi =  ex1["pi"];
//     spdlog::info("Json contains: {}", ex1.dump());
//     spdlog::info("Pi is {}\n", pi);
//     mgm::io::safe_to_disk(sol, parser.output_path);
    
//     spdlog::info("----SOLVER TEST----");
//     spdlog::info("Building solver");
//     mgm::QAPSolver s(model->models[mgm::GmModelIdx(0,1)], 10, 10, 10);
//     spdlog::info("Running solver");
//     auto solution = s.run();

//     spdlog::info("GM Solution: {}", solution.labeling);
// }

// void test_mgm_solver(int argc, char **argv) {
//     ArgParser parser(argc, argv);
//     mgm::init_logger(parser.output_path);

//     auto mgmModel = mgm::io::parse_dd_file(parser.input_file);
//     auto model = std::make_shared<mgm::MgmModel>(std::move(mgmModel));

//     print_mem_usage();

//     auto solver = mgm::SequentialGenerator(model);

//     auto order = mgm::SequentialGenerator::matching_order::random;
    
//     auto search_order = solver.init_generation_sequence(order);
//     solver.generate();

//     print_mem_usage();
    
//     auto clique_manager = solver.export_CliqueManager();
//     //auto local_searcher = mgm::LocalSearcher(clique_manager, search_order, model);
//     auto local_searcher = mgm::LocalSearcherParallel(clique_manager, model);
//     local_searcher.search();

//     auto sol = local_searcher.export_solution();

//     mgm::io::safe_to_disk(sol, parser.output_path);
// }

// void compare_local_searcher(int argc, char **argv) {
//     ArgParser parser(argc, argv);
//     mgm::init_logger(parser.output_path);

//     auto mgmModel = mgm::io::parse_dd_file(parser.input_file);
//     auto model = std::make_shared<mgm::MgmModel>(std::move(mgmModel));

//     print_mem_usage();

//     auto solver = mgm::SequentialGenerator(model);

//     auto order = mgm::SequentialGenerator::matching_order::random;

//     auto search_order = solver.init_generation_sequence(order);
//     solver.generate();
    
//     auto clique_manager = solver.export_CliqueManager();
    
//     {
//         auto begin = std::chrono::steady_clock::now();
//         auto local_searcher = mgm::LocalSearcher(clique_manager, search_order, model);
//         local_searcher.search();

//         auto sol = local_searcher.export_solution();
//         auto end = std::chrono::steady_clock::now();
//         auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
//         spdlog::info("Localsearch took {}ms", diff);
//         spdlog::info("Energy: {}\n", sol.evaluate());
//     }
//     {
//         auto begin = std::chrono::steady_clock::now();
//         auto local_searcher = mgm::LocalSearcherParallel(clique_manager, model);
//         local_searcher.search();

//         auto sol = local_searcher.export_solution();
//         auto end = std::chrono::steady_clock::now();
//         auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
//         spdlog::info("Parallel Localsearch took {}ms", diff);
//         spdlog::info("Energy: {}", sol.evaluate());
//     }
    
//     //mgm::io::safe_to_disk(sol, parser.outPath);
// }

// void test_sequential_generator(int argc, char **argv) {
//     ArgParser parser(argc, argv);
//     mgm::init_logger(parser.output_path);

//     auto mgmModel = mgm::io::parse_dd_file(parser.input_file);
//     auto model = std::make_shared<mgm::MgmModel>(std::move(mgmModel));

//     print_mem_usage();

//     auto solver = mgm::SequentialGenerator(model);

//     auto order = mgm::SequentialGenerator::matching_order::random;
//     auto search_order = solver.init_generation_sequence(order);
    
//     solver.generate();
    
//     auto sol = solver.export_solution();

//     mgm::io::safe_to_disk(sol, parser.output_path);

// }

// void test_parallel_generator(int argc, char **argv) {
//     ArgParser parser(argc, argv);
//     mgm::init_logger(parser.output_path);

//     auto mgmModel = mgm::io::parse_dd_file(parser.input_file);
//     auto model = std::make_shared<mgm::MgmModel>(std::move(mgmModel));

//     print_mem_usage();

//     auto solver = mgm::ParallelGenerator(model);
//     solver.generate();
    
//     auto sol = solver.export_solution();

//     mgm::io::safe_to_disk(sol, parser.output_path);
// }

// void test_cli11(int argc, char **argv) {
//     ArgParser parser(argc, argv);
//     mgm::init_logger(parser.output_path);
// }

// void compare_parallel_generator(int argc, char **argv) {
//     ArgParser parser(argc, argv);

//     mgm::init_logger(parser.output_path);

//     auto mgmModel = mgm::io::parse_dd_file(parser.input_file);
//     auto model = std::make_shared<mgm::MgmModel>(std::move(mgmModel));

//     print_mem_usage();

//     spdlog::info("Starting Sequential Generator");
//     auto solver = mgm::SequentialGenerator(model);
//     auto order = mgm::SequentialGenerator::matching_order::random;
//     auto search_order = solver.init_generation_sequence(order);
    
//     solver.generate();
//     spdlog::info("Finished Sequential Generator");
    
//     omp_set_num_threads(1);
//     spdlog::info("Starting Parallel Generator - 1 Thread");
//     auto solver_parallel = mgm::ParallelGenerator(model);
//     solver_parallel.generate();

//     spdlog::info("Finished Parallel Generator - 1 Threads");

//     omp_set_num_threads(3);
//     spdlog::info("Starting Parallel Generator - 3 Thread");
//     solver_parallel = mgm::ParallelGenerator(model);
//     solver_parallel.generate();

//     spdlog::info("Finished Parallel Generator - 3 Threads");

//     omp_set_num_threads(6);
//     spdlog::info("Starting Parallel Generator - 6 Thread");
//     solver_parallel = mgm::ParallelGenerator(model);
//     solver_parallel.generate();

//     spdlog::info("Finished Parallel Generator - 6 Threads");

//     omp_set_num_threads(12);
//     spdlog::info("Starting Parallel Generator - 12 Thread");
//     solver_parallel = mgm::ParallelGenerator(model);
//     solver_parallel.generate();    
//     spdlog::info("Finished Parallel Generator - 7 Thread");
    
//     omp_set_num_threads(7);
//     spdlog::info("Starting Parallel Generator - 7 Thread");
//     solver_parallel = mgm::ParallelGenerator(model);
//     solver_parallel.generate();

//     spdlog::info("Finished Parallel Generator - 7 Threads");
// }


// void test_qpbo(int argc, char **argv) {
//     ArgParser parser(argc, argv);

//     auto solver = qpbo::QPBO<double>(0,0);
//     int num_nodes = 500;

//     solver.AddNode(num_nodes);

//     for(int i = 0; i < num_nodes; i++) {
//         solver.AddUnaryTerm(i, 10, 20);
//     }
//     int counter = 0;
//     for(int i = 0; i < num_nodes; i++) {
//         for(int ii = i+1; ii < num_nodes; ii++) {
//             solver.AddPairwiseTerm(i, ii, 0, 10, 10, 0);
//             counter++;
//         }
//     }

//     int n = solver.GetMaxEdgeNum();

//     cout << "Added edges: " << counter << endl;
//     cout << "Max edge num: " << n << endl;
// }

// void test_ab_swap(int argc, char **argv) {
//     ArgParser parser(argc, argv);
//     mgm::init_logger(parser.output_path);

//     auto mgmModel = mgm::io::parse_dd_file(parser.input_file);
//     auto model = std::make_shared<mgm::MgmModel>(std::move(mgmModel));

//     print_mem_usage();

//     auto solver = mgm::SequentialGenerator(model);
//     auto order = mgm::SequentialGenerator::matching_order::random;
//     auto search_order = solver.init_generation_sequence(order);
//     solver.generate();
//     auto ab_optimizer = mgm::ABOptimizer(solver.export_CliqueTable(), model);
//     ab_optimizer.search();
//     auto sol = ab_optimizer.export_solution();

//     mgm::io::safe_to_disk(sol, parser.output_path);
// }

// void test_hashmap(int argc, char **argv) {
//     ArgParser parser(argc, argv);
//     mgm::init_logger(parser.output_path);

//     auto mgmModel = mgm::io::parse_dd_file(parser.input_file);
//     auto model = std::make_shared<mgm::MgmModel>(std::move(mgmModel));

//     print_mem_usage();
//     auto & map = model->models.at(mgm::GmModelIdx(0,1))->costs->all_edges();

//     double lf = map.load_factor();
//     cout << "Average load factor: " << lf << endl;

//     std::vector<size_t> bucket_size;
//     int no_buckets = map.bucket_count();
//     bucket_size.reserve(no_buckets);

//     int no_empty = 0;

//     for (int i = 0; i < no_buckets; i++) {
//         auto size = map.bucket_size(i);
//         if (size == 0) 
//             no_empty++;

//         bucket_size.push_back(size);
//     }
//     std::sort(bucket_size.begin(), bucket_size.end());

//     spdlog::info("{}", bucket_size);
//     spdlog::info("No Empty: {}", no_empty);
// }

int main(int argc, char **argv) {
    ArgParser argparser;
    ArgParser::Arguments args = argparser.parse(argc, argv);

    #ifndef NDEBUG
        spdlog::warn("RUNNING IN DEBUG MODE");
    #endif

    mgm::init_logger(args.output_path, args.output_filename);
    auto r = Runner(args);
    auto solution = r.run();

    mgm::io::safe_to_disk(solution, args.output_path, args.output_filename);

    return 0;

    //testing(argc, argv);
    //test_mgm_solver(argc, argv);
    //compare_local_searcher(argc, argv);
    //test_sequential_generator(argc, argv);
    //compare_parallel_generator(argc, argv);
    //test_cli11(argc, argv);
    //test_qpbo(argc, argv);
    //test_ab_swap(argc, argv);
    //test_hashmap(argc, argv);
}