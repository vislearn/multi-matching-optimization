from __future__ import annotations
import enum
import pylibmgm
import typing

__all__ = ['OptimizationLevel', 'solve_gm', 'solve_mgm', 'solve_mgm_pairwise', 'solve_mgm_parallel', 'synchronize_solution']

class OptimizationLevel(enum.Enum):
    """Optimization levels for the MGM solver.

    Attributes
    ----------
    FAST : OptimizationLevel
        Constructs an initial solution, without any local search.
        Choose if you need approximate solutions as quick as possible.

    DEFAULT : OptimizationLevel
        Constructs the solution and applies the GM local search.
        Strikes a balance between speed and solution quality.

    EXHAUSTIVE : OptimizationLevel
        Constructs the solution then iterates between GM local search and SWAP local search until no further improvement is found.
        The best we can currently do. May converge slowly for large problems.
    """
    FAST: typing.ClassVar[OptimizationLevel]  # value = <OptimizationLevel.FAST: 0>
    DEFAULT: typing.ClassVar[OptimizationLevel]  # value = <OptimizationLevel.DEFAULT: 1>
    EXHAUSTIVE: typing.ClassVar[OptimizationLevel]  # value = <OptimizationLevel.EXHAUSTIVE: 2>

def solve_gm(gm_model: pylibmgm.GmModel) -> pylibmgm.GmSolution:
    """Optimize a given GM model with the Fusion moves solver.

    Parameters
    ----------
    gm_model : GmModel
        Model to be optimized over.
    
    Returns
    -------
    GmSolution
        The optimized solution.
    
    References
    ----------
    [1] Fusion Moves for Graph Matching, Lisa Hutschenreiter, Stefan Haller, Lorenz Feineis, Carsten Rother, Dagmar Kainmüller, Bogdan Savchynskyy
    Proceedings of the IEEE/CVF International Conference on Computer Vision (ICCV), 2021, https://arxiv.org/abs/2101.12085
    """
    ...

def solve_mgm(model: pylibmgm.MgmModel, opt_level: OptimizationLevel = OptimizationLevel.EXHAUSTIVE) -> pylibmgm.MgmSolution:
    """Optimize a given MGM model with GREEDA.

    Parameters
    ----------
    model : MgmModel
        Model to be optimized over.
    opt_level : OptimizationLevel, optional
        Choose an optimization level to balance speed against solution quality.
    
    Returns
    -------
    MgmSolution
        The optimized solution.

    References
    ----------
    Max Kahl, Sebastian Stricker, Lisa Hutschenreiter, Florian Bernard, Carsten Rother, Bogdan Savchynskyy. Towards Optimizing Large-Scale Multi-Graph Matching in Bioimaging. Proceedings of the Computer Vision and Pattern Recognition Conference (CVPR), 2025.
    """
    ...

def solve_mgm_pairwise(mgm_model: pylibmgm.MgmModel) -> pylibmgm.MgmSolution:
    """Optimize the pairwise GM problems within the given MGM model independently.

    WARNING: The returned solution will very likely not be cycle-consistent.

    Parameters
    ----------
    mgm_model : MgmModel
        Model to be optimized over.
    
    Returns
    -------
    MgmSolution
        The optimized solution (likely not cycle-consistent).
    """
    ...

def solve_mgm_parallel(model: pylibmgm.MgmModel, opt_level: OptimizationLevel = OptimizationLevel.EXHAUSTIVE, nr_threads: int = 4) -> pylibmgm.MgmSolution:
    """Optimize a given MGM model with GREEDA. Use parallel construction and GM local search.

    Parameters
    ----------
    model : MgmModel
        Model to be optimized over.
    opt_level : OptimizationLevel, optional
        Choose an optimization level to balance speed against solution quality.
    nr_threads : int, optional
        Number of threads to use for parallel construction and GM local search.
        Is passed internally to omp_set_num_threads. Default is 4.
    
    Returns
    -------
    MgmSolution
        The optimized solution.
    """
    ...

def synchronize_solution(model: pylibmgm.MgmModel, solution: pylibmgm.MgmSolution, feasible: bool = True, iterations: int = 3, opt_level: OptimizationLevel = OptimizationLevel.EXHAUSTIVE) -> pylibmgm.MgmSolution:
    """Use GREEDA as a synchronization algorithm.

    The algorithm takes in a cycle inconsistent solution and returns a consistent one.

    Parameters
    ----------
    model : MgmModel
        Model to be optimized over.
    solution : MgmSolution
        Initial (cycle-inconsistent) solution of the given model.
    feasible : bool, optional
        If True, the synchronization problem will not consider forbidden matchings i.e. unspecified costs of model.
        As such, a solution remains valid under sparsity assumptions and contain only matchings for which a cost factor is contained in the model.
        However, most other synchronization algorithms can not account for sparsity. As such, setting feasible to False
        allows the algorithm to consider the same unrestricted search space as other synchronization algorithms. Default is True.
    iterations : int, optional
        Specify, how often to run the algorithm. Only the best solution of the specified number of iterations will be returned. Default is 3.
    opt_level : OptimizationLevel, optional
        Choose an optimization level to balance speed against solution quality.
    
    Returns
    -------
    MgmSolution
        The synchronized solution.
    """
    ...
