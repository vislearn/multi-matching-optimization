#include <iostream>

// Logging
#include "spdlog/spdlog.h"

// project
#include "argparser.hpp"
#include "logger.hpp"

int main(int argc, char **argv) {
    ArgParser parser(argc, argv);
    init_logger(parser.outPath);
    
}