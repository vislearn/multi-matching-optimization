#include <stdio.h>
#include <qpbo.h>
#include <mpopt/qap.h>

#include <iostream>

//
// main is where all program execution starts
//

int test_qpbo() {

  auto solver = new qpbo::QPBO<double>(0,0);

  solver->AddNode(2);

  solver->AddUnaryTerm(0, 10, 20);
  solver->AddUnaryTerm(1, 25,  5);
  solver->AddPairwiseTerm(0, 1, 0, 10, 10, 0);

  solver->Solve();
  solver->ComputeWeakPersistencies();

  std::cout << 0 << ": " << solver->GetLabel(0) << std::endl;
  std::cout << 0 << ": " << solver->GetLabel(1) << std::endl;
  return 0;
}

mpopt_qap_solver* construct_solver() {
  auto solver = mpopt_qap_solver_create();
  auto graph = mpopt_qap_solver_get_graph(solver);
  
  return solver;
}

int test_mpopt() {
  std::cout << "Creating Solver" << std::endl;
  auto solver = construct_solver();

  mpopt_qap_solver_set_fusion_moves_enabled(solver, true);
  mpopt_qap_solver_set_local_search_enabled(solver, true);
  mpopt_qap_solver_set_dual_updates_enabled(solver, true);
  mpopt_qap_solver_set_grasp_alpha(solver, 0.25);
  mpopt_qap_solver_use_grasp(solver);


  std::cout << "Running Solver" << std::endl;
  //mpopt_qap_solver_run(solver, 10, 100, 10);

  std::cout << "Destroying Solver" << std::endl;
  mpopt_qap_solver_destroy(solver);
  //mpopt_qap_solver_finalize(solver);
  std::cout << "yep" << std::endl;
  return 0;
}

int main(int argc, char **argv) {
  
  int result = test_qpbo();
  result = test_mpopt();
  return result;
}