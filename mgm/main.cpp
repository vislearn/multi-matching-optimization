#include <iostream>
#include <unistd.h>
#include <ios>
#include <fstream>
#include <string>
#include <memory>

#include <chrono>

// Logging
#include <spdlog/spdlog.h>

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
}