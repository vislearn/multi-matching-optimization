#ifndef LIBMGM_SYNCHRONIZATION_HPP
#define LIBMGM_SYNCHRONIZATION_HPP

#include "solution.hpp"
#include "multigraph.hpp"

namespace mgm {

std::shared_ptr<MgmModel> build_sync_problem(std::shared_ptr<MgmModel> model, MgmSolution& solution, bool feasible=true);

namespace details {

std::shared_ptr<GmModel>  create_feasible_sync_model(GmSolution& solution);
std::shared_ptr<GmModel>  create_infeasible_sync_model(GmSolution& solution);

}
}

#endif