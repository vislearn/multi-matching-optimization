#include <iostream>
#include <filesystem>
#include <getopt.h>

#include "argparser.hpp"

/*
* Copied mostly from the gnu getopt_long documentation example.
*
* FIXME: Maybe just use boost library. Required Arguments not throwing error when missing, currently.
*/
const option ArgParser::cli_options[] = {
                {"input", required_argument, 0, 'i'},
                {"output", required_argument, 0, 'o'},
                {0, 0, 0, 0} // required by getopt.h  to indicate end of array
            };

ArgParser::ArgParser(int argc, char **argv) {
    parse(argc, argv);

    inputFile   = fs::absolute(inputFile);
    outPath  = fs::absolute(outPath);

    std::cout << "### Arguments passed ###" << std::endl;
    std::cout << "Input file: "     << inputFile     << std::endl;
    std::cout << "Output file: "    << outPath    << std::endl;
    std::cout << "########################" << std::endl;
}

void ArgParser::parse(int argc, char **argv) {
    int c;
    while (1)
    {
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, "i:o:", cli_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1) {
            break;
        }

        switch (c)  {
            case 0:
                /* If this option set a flag, do nothing else now. */
                if (cli_options[option_index].flag != 0)
                    break;
                printf("option %s", cli_options[option_index].name);
                if (optarg)
                    printf(" with arg %s", optarg);
                printf("\n");
                break;

            case 'i':
                this->inputFile = optarg;
                break;

            case 'o':
                this->outPath = optarg;
                break;

            case '?':
                /* getopt_long already printed an error message. */
                break;

            default:
                abort();
        }
    }

    /* Print any remaining command line arguments (not options). */
    if (optind < argc)
    {
        printf("non-option ARGV-elements: ");
        while (optind < argc)
            printf("%s ", argv[optind++]);
        putchar('\n');
    }
}