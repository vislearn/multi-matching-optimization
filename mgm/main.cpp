// Logging
#include <spdlog/spdlog.h>

#include <libmgm/mgm.hpp>

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