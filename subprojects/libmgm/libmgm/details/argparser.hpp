#ifndef LIBMGM_ARGPARSER_HPP
#define LIBMGM_ARGPARSER_HPP

#include <iostream>
#include <filesystem>
#include <getopt.h>

namespace fs = std::filesystem;

class ArgParser {
    public:
        ArgParser(int argc, char **argv);

        fs::path inputFile;
        fs::path outPath;
    
    private:
        const static option cli_options[];
        void parse(int argc, char **argv);
};

#endif