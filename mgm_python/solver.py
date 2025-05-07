import pylibmgm as lib

from enum import Enum
import logging
from math import inf as INFINITY

LOGGER = logging.getLogger("libmgm.interface")

class OptimizationLevel(Enum):
    FAST = 0            # Only construct solution, no local search
    DEFAULT = 1         # Construction + GM local search
    EXHAUSTIVE = 2      # Construction + GM local search <-> SWAP local search

def solve_mgm(model, opt_level = OptimizationLevel.DEFAULT):
    LOGGER.info("Solving MGM")
    solver = lib.SequentialGenerator(model)
    order = solver.init(lib.MgmGenerator.matching_order.random)
    solution = solver.generate()

    if opt_level == OptimizationLevel.FAST:
        return solution
    
    # First local search
    gm_searcher = lib.LocalSearcher(model, order)
    gm_searcher.search(solution)

    if opt_level == OptimizationLevel.DEFAULT:
        return solution
   
    # OptimizationLevel.EXHAUSTIVE
    swap_searcher = lib.ABOptimizer(model)
    i = 0

    improved = True
    while improved:
        improved = swap_searcher.search(solution)
        LOGGER.info(f"Exhaustive local search. Iteration {i+1}")
        if improved: 
            improved = gm_searcher.search(solution)
        else:
            return solution
        i += 1
    
    return solution
    
def solve_mgm_parallel(model, opt_level = OptimizationLevel.DEFAULT, nr_threads=4):
    lib.omp_set_num_threads(nr_threads)

    LOGGER.info("Solving MGM")
    solver = lib.ParallelGenerator(model)
    order = solver.init(lib.MgmGenerator.matching_order.random)
    solution = solver.generate()
    LOGGER.info("Solution energy: " + str(solution.evaluate()))

    if opt_level == OptimizationLevel.FAST:
        return solution
    
    # First local search
    gm_searcher = lib.LocalSearcherParallel(model)
    gm_searcher.search(solution)

    if opt_level == OptimizationLevel.DEFAULT:
        return solution
    
    # OptimizationLevel.EXHAUSTIVE
    swap_searcher = lib.ABOptimizer(model)

    improved = True
    i = 0
    while improved:
        LOGGER.info(f"Exhaustive local search. Iteration {i+1}")
        improved = swap_searcher.search(solution)
        if improved: 
            improved = gm_searcher.search(solution)
        else:
            return solution
        i += 1
    
    return solution

def synchronize_solution(model, solution, feasible=True, iterations = 3, opt_level = OptimizationLevel.DEFAULT):
    LOGGER.info(f"Building synchronization problem.")
    sync_model = lib.build_sync_problem(model, solution, feasible)

    LOGGER.info(f"Solving synchronization problem. Running {iterations} iterations.")
    best_solution = None
    best_obj = INFINITY
    for i in range(iterations):
        LOGGER.debug(f"Synchronization: Iteration {i+1}.")
        sync_solution = solve_mgm(sync_model, opt_level)
        obj = sync_solution.evaluate()
        if (obj < best_obj):
            LOGGER.debug(f"Synchronization: Found new best solution. Iteration: {i+1}. Objective: {obj}")
            best_obj = obj
            best_solution = sync_solution

    return best_solution

def solve_gm(gm_model):
    if gm_model.no_edges() == 0:
        solver = lib.LAPSolver(gm_model)
    else:
        solver = lib.QAPSolver(gm_model)

    return solver.run()

def solve_mgm_pairwise(mgm_model):
    LOGGER.info(f"Solving pairwise problems independently.")

    # Solve pairwise graph matchings
    solution = lib.MgmSolution(mgm_model)

    indices = sorted(mgm_model.models.keys())
    total = len(indices)
    interval = total / 5
    i = 0
    for gm_idx in indices:
        model = mgm_model.models[gm_idx]
        
        s = solve_gm(model)
        solution.set_solution(s)
        i += 1
        if (i % interval == 0):
            LOGGER.info(f"Progress: {i}/{total} pairs solved.")

    return solution