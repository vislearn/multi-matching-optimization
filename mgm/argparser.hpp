#ifndef MGM_ARGPARSER_HPP
#define MGM_ARGPARSER_HPP

#include <iostream>
#include <filesystem>
#include <getopt.h>
#include <CLI/CLI.hpp>

namespace fs = std::filesystem;

class ArgParser {
    public:
        enum optimization_mode {
            fast,
            incremental,
            balanced,
            optimal
        };
        struct Arguments {
            fs::path input_file;
            fs::path output_path;
            std::string output_filename = "mgm";


            int nr_threads = 1;
            int incremental_set_size;

            optimization_mode  mode = optimal;
        };

    //TODO: Consider making this a static function
        Arguments parse(int argc, char **argv);
    
    private:
        std::map<std::string, ArgParser::optimization_mode> mode_map   {{"fast", optimization_mode::fast}, 
                                                                        {"incremental", optimization_mode::incremental},
                                                                        {"balanced", optimization_mode::balanced}, 
                                                                        {"optimal", optimization_mode::optimal}};

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
        CLI::Option* optimization_mode_option  = app.add_option("--mode", this->args.mode)
            ->description("Set speed and quality of the optimizer.\n"
                            "fast: Parallel generation.\n"
                            "balanced: Parallel generation -> parallel local search.\n"
                            "optimal: Parallel generation -> parallel local search <-> swap local search")
            ->required()
            ->transform(CLI::CheckedTransformer(mode_map, CLI::ignore_case));
        
        [[maybe_unused]]		
        CLI::Option* incremental_set_size_option  = app.add_option("--set-size", this->args.incremental_set_size)
            ->description("Subset size for incremenetal generation");
            
        // Optional options
        [[maybe_unused]] 		
        CLI::Option* nr_threads_opt     = app.add_option("-t,--threads", this->args.nr_threads)
            ->description("Number of threads to use. Upper limit defined by OMP_NUM_THREADS environment variable.");
};

#endif