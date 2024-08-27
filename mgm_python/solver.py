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
    order = solver.init(lib.SequentialGenerator.matching_order.random)
    solver.generate()

    if opt_level == OptimizationLevel.FAST:
        return solver.export_solution()
    
    # First local search
    cliquemanager = solver.export_cliquemanager()
    gm_searcher = lib.LocalSearcher(cliquemanager, order, model)
    gm_searcher.search()

    if opt_level == OptimizationLevel.DEFAULT:
        return gm_searcher.export_solution()
    else: # OptimizationLevel.DEFAULT
        cliquemanager = gm_searcher.export_cliquemanager()
        cliquetable = gm_searcher.export_cliquetable()
        return _run_exhaustive_ls(cliquemanager, cliquetable, order, model)
    
def _run_exhaustive_ls(clique_manager, cliquetable, order, model):
    gm_searcher = None
    swap_searcher = None

    improved = True
    i = 0
    while (improved):
        LOGGER.info(f"Exhaustive local search. Iteration {i+1}")
        # SWAP-LS
        swap_searcher = lib.ABOptimizer(cliquetable, model)
        improved = swap_searcher.search()
        
        # GM-LS
        if (improved):
            cliquetable = swap_searcher.export_cliquetable()
            clique_manager.reconstruct_from(cliquetable)
            
            gm_searcher = lib.LocalSearcher(clique_manager, order, model)
            improved = gm_searcher.search()

            cliquetable = gm_searcher.export_cliquetable()
        else:
            return swap_searcher.export_solution()
        
        i += 1
    
    return gm_searcher.export_solution()

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
        solution[gm_idx] = s
        i += 1
        if (i % interval == 0):
            LOGGER.info(f"Progress: {i}/{total} pairs solved.")

    return solution