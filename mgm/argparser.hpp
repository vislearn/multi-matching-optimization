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
            balanced,
            optimal
        };
        struct Arguments {
            fs::path input_file;
            fs::path output_path;

            int nr_threads = 1;

            optimization_mode  mode = optimal;
        };

    //TODO: Consider making this a static function
        Arguments parse(int argc, char **argv);
    
    private:
        std::map<std::string, ArgParser::optimization_mode> mode_map   {{"fast", optimization_mode::fast}, 
                                                                        {"balanced", optimization_mode::balanced}, 
                                                                        {"optimal", optimization_mode::optimal}};

        Arguments args;

        CLI::App app{"Multi-Graph Matching Optimizer"};

        // Required options
        CLI::Option* input_file_option   = app.add_option("-i,--infile", this->args.input_file, "Path to .dd input file.")
            ->required();
        CLI::Option* output_path_option  = app.add_option("-o,--outpath", this->args.output_path, "Path to output directory.")
            ->required();        
        
        CLI::Option* optimization_mode_option  = app.add_option("--mode", this->args.mode, "Set speed and quality of the optimizer.\n fast: Parallel generation.\n balanced: Parallel generation -> parallel local search.\n optimal: Parallel generation -> parallel local search <-> swap local search")
            ->required()
            ->transform(CLI::CheckedTransformer(mode_map, CLI::ignore_case));
            
        // Optional options
        CLI::Option* nr_threads_opt     = app.add_option("-t,--threads", this->args.nr_threads, "Number of threads to use. Upper limit defined by OMP_NUM_THREADS environment variable.");
    
};

#endif