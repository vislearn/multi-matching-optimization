#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <CLI/CLI.hpp>
#include <omp.h>

#include "argparser.hpp"

ArgParser::ArgParser(int argc, char **argv) {
    parse(argc, argv);

    inputFile   = fs::absolute(inputFile);
    outPath     = fs::absolute(outPath);
    omp_set_num_threads(this->nr_threads);

    std::cout << "### Arguments passed ###" << std::endl;
    std::cout << "Input file: "     << inputFile     << std::endl;
    std::cout << "Output file: "    << outPath    << std::endl;
    std::cout << "########################" << std::endl;
}

void ArgParser::parse(int argc, char **argv) {
    CLI::App app{"Multi-Graph Matching Optimizer"};

    auto in_opt = app.add_option("-i,--infile", this->inputFile, "Path to .dd input file.");
    in_opt->required();

    auto out_opt = app.add_option("-o,--outpath", this->outPath, "Path to output directory.");
    out_opt->required();

    auto nr_threads_opt = app.add_option("-t,--threads", this->nr_threads, "Number of threads to use. Upper limit defined by OMP_NUM_THREADS environment variable.");
    nr_threads_opt->check(CLI::Range(1,omp_get_max_threads()));

    try {
        app.parse((argc), (argv));
    } catch(const CLI::ParseError &e) {
        std::exit((app).exit(e));
    }
}