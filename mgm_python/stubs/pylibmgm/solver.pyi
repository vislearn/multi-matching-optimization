"""Type stubs for pylibmgm.solver module.

Documentation is maintained in mgm_python/solver.py (the source of truth).
This file contains only type signatures for IDE and type checker support.
"""
from __future__ import annotations
import enum
import pylibmgm
import typing

__all__ = ['OptimizationLevel', 'solve_gm', 'solve_mgm', 'solve_mgm_pairwise', 'solve_mgm_parallel', 'synchronize_solution']

class OptimizationLevel(enum.Enum):
    FAST: typing.ClassVar[OptimizationLevel]
    DEFAULT: typing.ClassVar[OptimizationLevel]
    EXHAUSTIVE: typing.ClassVar[OptimizationLevel]

def solve_gm(gm_model: pylibmgm.GmModel) -> pylibmgm.GmSolution: ...

def solve_mgm(model: pylibmgm.MgmModel, opt_level: OptimizationLevel = ...) -> pylibmgm.MgmSolution: ...

def solve_mgm_pairwise(mgm_model: pylibmgm.MgmModel) -> pylibmgm.MgmSolution: ...

def solve_mgm_parallel(model: pylibmgm.MgmModel, opt_level: OptimizationLevel = ..., nr_threads: int = ...) -> pylibmgm.MgmSolution: ...

def synchronize_solution(model: pylibmgm.MgmModel, solution: pylibmgm.MgmSolution, feasible: bool = ..., iterations: int = ..., opt_level: OptimizationLevel = ...) -> pylibmgm.MgmSolution: ...
