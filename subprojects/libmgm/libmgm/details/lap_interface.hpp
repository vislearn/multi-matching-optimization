#ifndef LIBMGM_LAP_INTERFACE_HPP
#define LIBMGM_LAP_INTERFACE_HPP

#include "multigraph.hpp"
#include "solution.hpp"

namespace mgm {

class LAPSolver {
    public:
        LAPSolver(std::shared_ptr<GmModel> model);
        
        GmSolution run();
    private:
        std::shared_ptr<GmModel> model;

        std::vector<double> costs;
        int nr_cols;
        int nr_rows;


};
}

#endif