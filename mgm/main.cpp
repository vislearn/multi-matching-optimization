#include <iostream>
#include <unistd.h>
#include <ios>
#include <fstream>
#include <string>
#include <memory>

// Logging
#include <spdlog/spdlog.h>
#include <fmt/ranges.h> // print vector

// json
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <libmgm/mgm.hpp>
#include "argparser.hpp"

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

void testing(int argc, char **argv) {
    ArgParser parser(argc, argv);
    mgm::init_logger(parser.outPath);

    spdlog::info("----PARSER TEST----");
    auto mgmModel = mgm::io::parse_dd_file(parser.inputFile);
    
    print_mem_usage();
    
    double u = mgmModel.models[mgm::GmModelIdx(0,1)]->costs->unary(0,0);
    spdlog::info("Model (0,1) contains cost (0,0) = {}", u);

    spdlog::info("----MODEL TEST----");
    auto model = std::make_shared<mgm::MgmModel>(std::move(mgmModel));
    
    auto sol = mgm::MgmSolution(model);

    spdlog::info("----SOLUTION TEST----");
    for (auto & [key, gm_sol] : sol.gmSolutions) {
        int no_left = gm_sol.model->graph1.no_nodes;
        int no_right = gm_sol.model->graph2.no_nodes;
        for (auto i = 0; i < no_left; i++) {
            gm_sol.labeling[i] = i;
            if (i >= no_right) {
                break;
            }
        }

        std::ostringstream oss;

        // Convert all but the last element to avoid a trailing ","
        std::copy(gm_sol.labeling.begin(), gm_sol.labeling.end()-1,
            std::ostream_iterator<int>(oss, ","));

        // Now add the last element with no delimiter
        oss << gm_sol.labeling.back();
        spdlog::info("G{}: {} nodes", gm_sol.model->graph1.id, gm_sol.model->graph1.no_nodes);
        spdlog::info("G{}: {} nodes", gm_sol.model->graph2.id, gm_sol.model->graph2.no_nodes);
        
        spdlog::info("L{}-{}: {}", key.first, key.second, oss.str());
    }
    spdlog::info("----JSON TEST----");
    json ex1 = json::parse(R"(
    {
        "pi": 3.141,
        "happy": true
    }
    )");
    
    double pi =  ex1["pi"];
    spdlog::info("Json contains: {}", ex1.dump());
    spdlog::info("Pi is {}\n", pi);
    mgm::io::safe_to_disk(sol, parser.outPath);
    
    spdlog::info("----SOLVER TEST----");
    spdlog::info("Building solver");
    mgm::QAPSolver s(model->models[mgm::GmModelIdx(0,1)], 10, 10, 10);
    spdlog::info("Running solver");
    auto solution = s.run();

    spdlog::info("GM Solution: {}", solution.labeling);
}

void test_mgm_solver(int argc, char **argv) {
    ArgParser parser(argc, argv);
    mgm::init_logger(parser.outPath);

    auto mgmModel = mgm::io::parse_dd_file(parser.inputFile);
    auto model = std::make_shared<mgm::MgmModel>(std::move(mgmModel));

    print_mem_usage();

    auto solver = mgm::MgmGenerator(model);

    auto order = mgm::MgmGenerator::generation_order::random;

    solver.generate(order);

    print_mem_usage();
    

    auto clique_manager = solver.export_CliqueManager();
    auto local_searcher = mgm::LocalSearcher(clique_manager, model);
    local_searcher.set_search_order_random();
    local_searcher.search();

    auto sol = local_searcher.export_solution();

    mgm::io::safe_to_disk(sol, parser.outPath);
}

int main(int argc, char **argv) {
    #ifndef NDEBUG
        spdlog::warn("RUNNING IN DEBUG MODE");
    #endif

    cout << "Test.." << endl;
    //testing(argc, argv);
    test_mgm_solver(argc, argv);

    return 0;
}