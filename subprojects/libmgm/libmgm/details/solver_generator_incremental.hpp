#ifndef LIBMGM_SOLVER_GENERATOR_INCREMENTAL_HPP
#define LIBMGM_SOLVER_GENERATOR_INCREMENTAL_HPP

#include "solver_generator_mgm.hpp"

namespace mgm {
    
class IncrementalGenerator : public SequentialGenerator {
    public:
        IncrementalGenerator(int subset_size, std::shared_ptr<MgmModel> model);
        
        MgmSolution generate() override;

    private:
        int subset_size;

        void improve();
};

}

#endif