from __future__ import annotations
import os
import pylibmgm
import typing
__all__ = ['export_dd_file', 'import_solution', 'parse_dd_file', 'parse_dd_file_gm', 'save_to_disk']

def export_dd_file(filepath: os.PathLike, model: pylibmgm.MgmModel) -> None:
    ...

def import_solution(solution_path: os.PathLike, model: pylibmgm.MgmModel) -> pylibmgm.MgmSolution:
    ...

def parse_dd_file(dd_file: os.PathLike, unary_constant: float = 0.0) -> pylibmgm.MgmModel:
    ...

def parse_dd_file_gm(gm_dd_file: os.PathLike, unary_constant: float = 0.0) -> pylibmgm.GmModel:
    ...

@typing.overload
def save_to_disk(filepath: os.PathLike, solution: pylibmgm.MgmSolution) -> None:
    ...

@typing.overload
def save_to_disk(filepath: os.PathLike, solution: pylibmgm.GmSolution) -> None:
    ...
