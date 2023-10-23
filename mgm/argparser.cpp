#include <iostream>
#include <filesystem>
#include <CLI/CLI.hpp>
#include <omp.h>
#include <map>

#include "argparser.hpp"

ArgParser::Arguments ArgParser::parse(int argc, char **argv) {

    try {
        this->app.parse((argc), (argv));

        this->args.input_file   = fs::absolute(this->args.input_file);
        this->args.output_path  = fs::absolute(this->args.output_path);
        if (*this->labeling_path_option)
            this->args.labeling_path  = fs::absolute(this->args.labeling_path);

        // For incremental generation, assert agreement between mode and set size option
        if (*this->incremental_set_size_option) {
            if(this->args.mode != this->optimization_mode::incremental)
                throw CLI::ValidationError("'incremental_set_size' option only available in 'incremental' mode.");
        }
        if (this->args.mode == this->optimization_mode::incremental) {
            if(!(*this->incremental_set_size_option))
                throw CLI::RequiredError("'incremental' mode: Option 'set_size'");
        }

        omp_set_num_threads(this->args.nr_threads);

        std::cout << "### Arguments passed ###" << std::endl;
        std::cout << "Input file: "         << this->args.input_file        << std::endl;
        std::cout << "Output folder: "      << this->args.output_path       << std::endl;
        if (*this->output_filename_option)
            std::cout << "Filename: "       << this->args.output_filename   << std::endl;
        std::cout << "########################" << std::endl;

    } catch(const CLI::ParseError &e) {
        std::exit((this->app).exit(e));
    }
    return this->args;
}