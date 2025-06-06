from __future__ import annotations
import enum
from enum import Enum
import logging as logging
import pylibmgm as lib
import typing

__all__ = ['OptimizationLevel', 'solve_gm', 'solve_mgm', 'solve_mgm_pairwise', 'solve_mgm_parallel', 'synchronize_solution']
class OptimizationLevel(enum.Enum):
    DEFAULT: typing.ClassVar[OptimizationLevel]  # value = <OptimizationLevel.DEFAULT: 1>
    EXHAUSTIVE: typing.ClassVar[OptimizationLevel]  # value = <OptimizationLevel.EXHAUSTIVE: 2>
    FAST: typing.ClassVar[OptimizationLevel]  # value = <OptimizationLevel.FAST: 0>
    
def solve_gm(gm_model):
    ...
def solve_mgm(model, opt_level = ...):
    ...
def solve_mgm_pairwise(mgm_model):
    ...
def solve_mgm_parallel(model, opt_level = ..., nr_threads = 4):
    ...
def synchronize_solution(model, solution, feasible = True, iterations = 3, opt_level = ...):
    ...
