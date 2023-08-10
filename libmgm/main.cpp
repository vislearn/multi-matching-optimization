#include <iostream>
#include <unistd.h>
#include <ios>
#include <fstream>
#include <string>
#include <memory>

// Logging
#include "spdlog/spdlog.h"

// project
#include "argparser.hpp"
#include "logger.hpp"
#include "multigraph.hpp"
#include "dd_parser.hpp"
#include "solution.hpp"

using namespace std;
void mem_usage(double& vm_usage, double& resident_set) {
   vm_usage = 0.0;
   resident_set = 0.0;
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
}

int main(int argc, char **argv) {

    ArgParser parser(argc, argv);
    init_logger(parser.outPath);

    auto mgmModel = parse_dd_file(parser.inputFile);
    
    double vm, rss;
    mem_usage(vm, rss);
    spdlog::info("++++++++++++++++++++++++++++");
    spdlog::info("--After Parsing input file--");
    spdlog::info("| Virtual Memory: {}MB", vm / 1024.0);
    spdlog::info("| Resident set size: {}MB",rss / 1024.0);
    spdlog::info("----------------------------");

    cout << mgmModel.models[GmModelIdx(0,1)]->costs->unary(0,0) << endl;

    auto model = std::make_shared<MgmModel>(std::move(mgmModel));
    
    auto sol = MgmSolution(model);

    for (auto & [key, gm_sol] : sol.gmSolutions) {
        int no_right = gm_sol.model->graph2.no_nodes;
        for (auto i = 0; i < no_right; i++) {
            gm_sol.labeling[i] = i;
        } 
        
        
        std::ostringstream oss;

        // Convert all but the last element to avoid a trailing ","
        std::copy(gm_sol.labeling.begin(), gm_sol.labeling.end()-1,
            std::ostream_iterator<int>(oss, ","));

        // Now add the last element with no delimiter
        oss << gm_sol.labeling.back();

        spdlog::info("L1: {}", oss.str());
    }


    return 0;
}