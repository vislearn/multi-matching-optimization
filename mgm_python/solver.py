import pylibmgm as lib

from enum import Enum
import logging
from math import inf as INFINITY

LOGGER = logging.getLogger("libmgm.interface")

class OptimizationLevel(Enum):
    """ Optimization levels for the MGM solver.


    Attributes
    ----------
    FAST: OptimizationLevel
        Constructs an initial solution, without any local search.
        Choose if you need approximate solutions as quick as possible.

    DEFAULT: OptimizationLevel
        Constructs the solution and applies the GM local search.
        Strikes a balance between speed and solution quality.

    EXHAUSTIVE: OptimizationLevel
        Constructs the solution then iterates between GM local search and SWAP local search until no further improvement is found.
        The best we can currently do. May converge slowly for large problems.

    """

    FAST = 0            # Only construct solution, no local search
    DEFAULT = 1         # Construction + GM local search
    EXHAUSTIVE = 2      # Construction + GM local search <-> SWAP local search

def solve_mgm(model, opt_level = OptimizationLevel.EXHAUSTIVE):
    """ Optimize a given MGM model with GREEDA.

    Parameters
    ----------
    model: :class:`pylibmgm.MgmModel`
        Model to be optimized over.

    opt_level: :class:`pylibmgm.OptimizationLevel`
        Choose an optimization level to balance speed against solution quality.

    Returns
    -------
    :class:`pylibmgm.MgmSolution`

    """

    LOGGER.info("Solving MGM")
    solver = lib.SequentialGenerator(model)
    order = solver.init(lib.MgmGenerator.matching_order.random)
    solution = solver.generate()

    if opt_level == OptimizationLevel.FAST:
        return solution
    
    # First local search
    gm_searcher = lib.GMLocalSearcher(model, order)
    gm_searcher.search(solution)

    if opt_level == OptimizationLevel.DEFAULT:
        return solution
   
    # OptimizationLevel.EXHAUSTIVE
    swap_searcher = lib.SwapLocalSearcher(model)
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
    
def solve_mgm_parallel(model, opt_level = OptimizationLevel.EXHAUSTIVE, nr_threads=4):
    """ Optimize a given MGM model with GREEDA. Use parallel construction and GM local search.

    Parameters
    ----------
    model: :class:`pylibmgm.MgmModel`
        Model to be optimized over.

    opt_level: :class:`pylibmgm.OptimizationLevel`
        Choose an optimization level to balance speed against solution quality.

    nr_threads: int, optional
        Number of threads to use for parallel construction and GM local search.
        Is passed internally to :class:`pylibmgm.omp_set_num_threads`.

    Returns
    -------
    :class:`pylibmgm.MgmSolution`

    """
    lib.omp_set_num_threads(nr_threads)

    LOGGER.info("Solving MGM")
    solver = lib.ParallelGenerator(model)
    order = solver.init(lib.MgmGenerator.matching_order.random)
    solution = solver.generate()
    LOGGER.info("Solution energy: " + str(solution.evaluate()))

    if opt_level == OptimizationLevel.FAST:
        return solution
    
    # First local search
    gm_searcher = lib.GMLocalSearcherParallel(model)
    gm_searcher.search(solution)

    if opt_level == OptimizationLevel.DEFAULT:
        return solution
    
    # OptimizationLevel.EXHAUSTIVE
    swap_searcher = lib.SwapLocalSearcher(model)

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

def synchronize_solution(model, solution, feasible=True, iterations = 3, opt_level = OptimizationLevel.EXHAUSTIVE):
    """ Use GREEDA as a synchronization algorithm

    The algorithm takes in a cycle inconsistent solution and returns a consistent one.

    Parameters
    ----------
    model: :class:`pylibmgm.MgmModel`
        Model to be optimized over.
    solution: :class:`pylibmgm.MgmSolution`
        Initial (cycle-inconsistent) solution of the given model.

    feasible: bool, optional
        If True, the synchronization problem will not consider forbidden matchings i.e. unspecified costs of model.
        As such, a solution remains valid under sparsity assumptions and contain only matchings for which a cost factor is contained in the model.
        However, most other synchronization algorithms can not account for sparsity. As such, setting feasible to False
        allows the algorithm to consider the same unrestricted search space as other synchronization algorithms.

    iterations: int, optional
        Specify, how often to run the algorithm. Only the best solution of the specified number of iterations will be returned.

    opt_level: :class:`pylibmgm.OptimizationLevel`, optional
        Choose an optimization level to balance speed against solution quality.

    Returns
    -------
    :class:`pylibmgm.MgmSolution` 

    """
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
    """ Optimize a given GM model with the Fusion moves solver.

    Parameters
    ----------
    gm_model: :class:`pylibmgm.GmModel`
        Model to be optimized over.
    Returns
    -------
    :class:`pylibmgm.GmSolution`

    References
    ----------
    [1] Fusion Moves for Graph Matching, Lisa Hutschenreiter, Stefan Haller, Lorenz Feineis, Carsten Rother, Dagmar Kainmüller, Bogdan Savchynskyy
    Proceedings of the IEEE/CVF International Conference on Computer Vision (ICCV), 2021, https://arxiv.org/abs/2101.12085

    """
    if gm_model.no_edges() == 0:
        solver = lib.LAPSolver(gm_model)
    else:
        solver = lib.QAPSolver(gm_model)

    return solver.run()

def solve_mgm_pairwise(mgm_model):
    """ Optimize the pairwise GM problems within the given MGM model independently.

    WARNING: The returned solution will very likely not be cycle-consistent.

    Parameters
    ----------
    mgm_model: :class:`pylibmgm.MgmModel`
        Model to be optimized over.
    Returns
    -------
    :class:`pylibmgm.MgmSolution` 


    """
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