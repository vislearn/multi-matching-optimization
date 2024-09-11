#ifndef MGM_ARGPARSER_HPP
#define MGM_ARGPARSER_HPP

#include <iostream>
#include <filesystem>
#include <CLI/CLI.hpp>

namespace fs = std::filesystem;

// TODO: Use CLI11 subcommands to make the optimization modes more approachable
// Separate functionality e.g.:
// ./mgm                (default mode, currently stored in --mode=optimal. Default case should be usable with as little arguments as possible)
// ./mgm generate       (Only generate, no local search)
// ./mgm improve        (local search ontop of a given labeling)
// ./mgm incremental    (Use incremental generation and enable set-size option)
// ... think of smartest way to arrange this
class ArgParser {
    public:
        enum optimization_mode {
            seq,
            par,
            inc,
            seqseq,
            seqpar,
            parseq,
            parpar,
            incseq,
            incpar,
            optimal,
            optimalpar,
            improve_swap,
            improve_qap,
            improve_qap_par,
            improveopt,
            improveopt_par,
            qap
        };
        struct Arguments {
            fs::path input_file;
            fs::path output_path;
            std::string output_filename = "mgm";

            fs::path labeling_path;

            int nr_threads = 1;
            int incremental_set_size;
            bool merge_one = false;
            unsigned long libmpopt_seed = 0;
            double unary_constant = 0.0;

            optimization_mode  mode = optimal;

            bool synchronize            = false;
            bool synchronize_infeasible = false;
        };

    //TODO: Consider making this a static function
        Arguments parse(int argc, char **argv);

    private:
        std::map<std::string, ArgParser::optimization_mode> mode_map   {{"seq", optimization_mode::seq},
                                                                        {"par", optimization_mode::par},
                                                                        {"inc", optimization_mode::inc},
                                                                        {"seqseq", optimization_mode::seqseq},
                                                                        {"seqpar", optimization_mode::seqpar},
                                                                        {"parseq", optimization_mode::parseq},
                                                                        {"parpar", optimization_mode::parpar},
                                                                        {"incseq", optimization_mode::incseq},
                                                                        {"incpar", optimization_mode::incpar},
                                                                        {"optimal", optimization_mode::optimal},
                                                                        {"optimalpar", optimization_mode::optimalpar},
                                                                        {"improve-swap", optimization_mode::improve_swap},
                                                                        {"improve-qap", optimization_mode::improve_qap},
                                                                        {"improve-qap-par", optimization_mode::improve_qap_par},
                                                                        {"improveopt", optimization_mode::improveopt},
                                                                        {"improveopt-par", optimization_mode::improveopt_par},
                                                                        {"qap", optimization_mode::qap}};

        Arguments args;

        CLI::App app{"Multi-Graph Matching Optimizer"};

        /*
        * [[maybe_unused]] supresses warning for unused variables.
        */
        // Required options
        [[maybe_unused]]
        CLI::Option* input_file_option   = app.add_option("-i,--infile", this->args.input_file)
            ->description("Path to .dd input file.")
            ->required();

        [[maybe_unused]] 		
        CLI::Option* output_path_option  = app.add_option("-o,--outpath", this->args.output_path)
            ->description("Path to output directory.")
            ->required();  

        [[maybe_unused]] 		
        CLI::Option* output_filename_option  = app.add_option("--name", this->args.output_filename)
            ->description("Base name for the resulting .json and .log files.");

        [[maybe_unused]]		
        CLI::Option* labeling_path_option  = app.add_option("-l,--labeling", this->args.labeling_path)
            ->description("Path to an existing solution file (json). Pass to improve upon this labeling.");

        [[maybe_unused]] 		
        CLI::Option* optimization_mode_option  = app.add_option("--mode", this->args.mode)
            ->description("Set speed and quality of the optimizer.\n"
                            "seq:        sequential generation\n"
                            "par:        parallel generation\n"
                            "inc:        incremental generation\n"
                            "seqseq:     sequential  generation -> sequential GM-LS\n"
                            "seqpar:     sequential  generation -> parallel   GM-LS\n"
                            "parseq:     parallel    generation -> sequential GM-LS\n"
                            "parpar:     parallel    generation -> parallel   GM-LS\n"
                            "incseq:     incremental generation -> sequential GM-LS\n"
                            "incpar:     incremental generation -> parallel   GM-LS\n"
                            "optimal:    sequential  generation -> Until conversion: (sequential GM-LS <-> swap local search)\n"
                            "optimalpar: parallel    generation -> Until conversion: (parallel   GM-LS <-> swap local search)\n"
                            "improve-swap:          improve a given labeling with SWAP-LS\n"
                            "improve-qap:           improve a given labeling with sequential GM-LS\n"
                            "improve-qap-par:       improve a given labeling with parallel GM-LS\n"
                            "improveopt:            improve a given labeling with alternating sequential GM-LS <-> SWAP-LS\n"
                            "improveopt-par:        improve a given labeling with alternating parallel GM-LS <-> SWAP-LS\n"
                            "qap:                   Single GM-Mode. Parse a graph matching .dd file and solve the qap problem.")
            ->required()
            ->transform(CLI::CheckedTransformer(mode_map, CLI::ignore_case));

        [[maybe_unused]]		
        CLI::Option* incremental_set_size_option  = app.add_option("--set-size", this->args.incremental_set_size)
            ->description("Subset size for incremenetal generation");

        [[maybe_unused]]		
        CLI::Option* merge_one_option  = app.add_flag("--merge-one", this->args.merge_one)
            ->description("In parallel local search, merge only the best solution. Do not try to merge other solutions as well.");

        // Optional options
        [[maybe_unused]] 		
        CLI::Option* nr_threads_opt     = app.add_option("-t,--threads", this->args.nr_threads)
            ->description("Number of threads to use. Upper limit defined by OMP_NUM_THREADS environment variable.");
                            
        [[maybe_unused]]		
        CLI::Option* libmpopt_seed_opt  = app.add_option("--libmpopt-seed", this->args.libmpopt_seed)
            ->description("Fix the random seed for the fusion moves graph matching solver of libmpopt. ");

        [[maybe_unused]]		
        CLI::Option* unary_constant_option  = app.add_option("--unary-constant", this->args.unary_constant)
            ->description("Constant to add to every assignment cost. Negative values nudges matchings to be more complete.");
            
        CLI::Option* synchronize_option  = app.add_flag("--synchronize", this->args.synchronize)
            ->description("Synchronize a cylce inconsistent solution.");

        [[maybe_unused]]		
        CLI::Option* synchronize_infeasible_option  = app.add_flag("--synchronize-infeasible", this->args.synchronize_infeasible)
            ->description("Synchronize a cylce inconsistent solution. Allow all (forbidden) matchings.")
            ->excludes(synchronize_option);
};

#endif