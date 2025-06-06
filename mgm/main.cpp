// Logging
#include <spdlog/spdlog.h>

#include <libmgm/mgm.hpp>

#include "argparser.hpp"
#include "runner.hpp"

using namespace std;

mgm::GmSolution solve_qap(ArgParser::Arguments args) {
    auto model = mgm::io::parse_dd_file_gm(args.input_file, args.unary_constant);

    if (model->no_edges() == 0) {
        spdlog::info("No edges. Constructing LAP solver...");
        mgm::LAPSolver solver(model);

        spdlog::info("Running LAP solver...");
        return solver.run();
    }
    else {
        spdlog::info("Constructing QAP solver...");
        mgm::QAPSolver solver(model);

        spdlog::info("Running QAP solver...");
        return solver.run();
    }
}

int main(int argc, char **argv) {
    ArgParser argparser;
    ArgParser::Arguments args = argparser.parse(argc, argv);

    auto loglevel = spdlog::level::level_enum::info;
    #ifndef NDEBUG
        loglevel = spdlog::level::level_enum::debug;
    #endif

    mgm::init_logger(args.output_path, args.output_filename, loglevel);

    #ifndef NDEBUG
        spdlog::warn("RUNNING IN DEBUG MODE");
    #endif

    // ONLY RUN QAP SOLVER
    if (args.mode == ArgParser::optimization_mode::qap) {
        auto solution = solve_qap(args);
        mgm::io::save_to_disk((args.output_path / args.output_filename), solution);
        return 0;
    }

    auto r = Runner(args);
    auto solution = r.run();

    mgm::io::save_to_disk((args.output_path / args.output_filename), solution);

    return 0;
}