#include <iostream>
#include <filesystem>
#include <CLI/CLI.hpp>
#include <omp.h>
#include <map>

#include "argparser.hpp"

ArgParser::Arguments ArgParser::parse(int argc, char **argv) {

    try {
        this->app.parse((argc), (argv));
    } catch(const CLI::ParseError &e) {
        std::exit((this->app).exit(e));
    }

    this->args.input_file   = fs::absolute(this->args.input_file);
    this->args.output_path  = fs::absolute(this->args.output_path);
    omp_set_num_threads(this->args.nr_threads);

    std::cout << "### Arguments passed ###" << std::endl;
    std::cout << "Input file: "     << this->args.input_file     << std::endl;
    std::cout << "Output file: "    << this->args.output_path    << std::endl;
    std::cout << "########################" << std::endl;

    return this->args;
}