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

        mgm::MgmSolution run_fast();
        mgm::MgmSolution run_balanced();
        mgm::MgmSolution run_optimal();
};

#endif