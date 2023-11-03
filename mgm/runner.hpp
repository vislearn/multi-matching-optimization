#ifndef MGM_RUNNER_HPP
#define MGM_RUNNER_HPP

#include <memory>
#include <filesystem>

#include <libmgm/mgm.hpp>

#include "argparser.hpp"

class Runner {
    public:
        Runner(ArgParser::Arguments args);

        mgm::MgmSolution run();

    private:
        std::shared_ptr<mgm::MgmModel> model;
        
        ArgParser::Arguments args;

        mgm::MgmSolution run_seqseq();
        mgm::MgmSolution run_seqpar();
        mgm::MgmSolution run_parseq();
        mgm::MgmSolution run_parpar();
        mgm::MgmSolution run_incseq();
        mgm::MgmSolution run_incpar();
        mgm::MgmSolution run_optimal();
        mgm::MgmSolution run_optimalpar();
        mgm::MgmSolution run_improveswap();
        mgm::MgmSolution run_improvels();
        mgm::MgmSolution run_improveopt();
};

#endif