#include <iostream>
#include <filesystem>
#include <CLI/CLI.hpp>
#include <omp.h>

#include <libmgm/mgm.hpp>

#include "argparser.hpp"

ArgParser::Arguments ArgParser::parse(int argc, char **argv) {

    try {
        this->app.parse((argc), (argv));

        this->args.input_file   = fs::absolute(this->args.input_file);
        this->args.output_path  = fs::absolute(this->args.output_path);
        if (*this->labeling_path_option) {
            this->args.labeling_path  = fs::absolute(this->args.labeling_path);

            if (this->args.mode != this->optimization_mode::improve_swap &&
                this->args.mode != this->optimization_mode::improve_qap &&
                this->args.mode != this->optimization_mode::improve_qap_par &&
                this->args.mode != this->optimization_mode::improveopt &&
                this->args.mode != this->optimization_mode::improveopt_par &&
                !this->args.synchronize &&
                !this->args.synchronize_infeasible)
                throw CLI::ValidationError("'labeling path' option only available in improve modes and for synchronization.");
        }

        // For incremental generation, assert agreement between mode and set size option
        if (*this->incremental_set_size_option) {
            if( this->args.mode != this->optimization_mode::inc && 
                this->args.mode != this->optimization_mode::incseq && 
                this->args.mode != this->optimization_mode::incpar)
                throw CLI::ValidationError("'incremental_set_size' option only available in 'incremental' mode.");
        }
        if (this->args.mode == this->optimization_mode::inc || 
            this->args.mode == this->optimization_mode::incseq || 
            this->args.mode == this->optimization_mode::incpar) {
            if(!(*this->incremental_set_size_option))
                throw CLI::RequiredError("'incremental' mode: Option 'set_size'");
        }
    
        mgm::QAPSolver::libmpopt_seed = this->args.libmpopt_seed;

        omp_set_num_threads(this->args.nr_threads);

        std::cout << "### Arguments passed ###" << std::endl;
        std::cout << "Input file: "             << this->args.input_file        << std::endl;
        std::cout << "Output folder: "          << this->args.output_path       << std::endl;
        std::cout << "Optimization mode: "      << this->args.mode              << std::endl;
        if (*this->labeling_path_option)
            std::cout << "Labeling path: "      << this->args.labeling_path     << std::endl;
        if (*this->output_filename_option)
            std::cout << "Output filename: "    << this->args.output_filename   << std::endl;
        if (*this->synchronize_option)
            std::cout << "Running as synchronization algorithm" << std::endl;
        else if (*this->synchronize_infeasible_option)
            std::cout << "Running as synchronization algorithm. Ignoring forbidden assignments." << std::endl;
        std::cout << "########################" << std::endl;



    } catch(const CLI::ParseError &e) {
        std::exit((this->app).exit(e));
    }
    return this->args;
}