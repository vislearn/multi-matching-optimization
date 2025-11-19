from __future__ import annotations
import os
import pylibmgm
import typing
__all__ = ['export_dd_file', 'import_solution', 'parse_dd_file', 'parse_dd_file_gm', 'save_to_disk']

def export_dd_file(filepath: os.PathLike, model: pylibmgm.MgmModel) -> None:
    """Export an MGM model to a .dd file.

    Parameters
    ----------
    filepath : os.PathLike
        Path where the .dd file will be written.
    model : MgmModel
        The MGM model to export.
    """
    ...

def import_solution(solution_path: os.PathLike, model: pylibmgm.MgmModel) -> pylibmgm.MgmSolution:
    """Import an MGM solution from disk.

    Parameters
    ----------
    solution_path : os.PathLike
        Path to the solution file.
    model : MgmModel
        The MGM model associated with the solution.

    Returns
    -------
    MgmSolution
        The imported solution.
    """
    ...

def parse_dd_file(dd_file: os.PathLike, unary_constant: float = 0.0) -> pylibmgm.MgmModel:
    """Parse a .dd file containing an MGM model.

    Parameters
    ----------
    dd_file : os.PathLike
        Path to the .dd file to parse.
    unary_constant : float, optional
        Constant value to add to all unary costs. Default is 0.0.

    Returns
    -------
    MgmModel
        The parsed MGM model.
    """
    ...

def parse_dd_file_gm(gm_dd_file: os.PathLike, unary_constant: float = 0.0) -> pylibmgm.GmModel:
    """Parse a .dd file containing a pairwise GM model.

    Parameters
    ----------
    gm_dd_file : os.PathLike
        Path to the .dd file to parse.
    unary_constant : float, optional
        Constant value to add to all unary costs. Default is 0.0.

    Returns
    -------
    GmModel
        The parsed GM model.
    """
    ...

@typing.overload
def save_to_disk(filepath: os.PathLike, solution: pylibmgm.MgmSolution) -> None:
    """Save a MGM solution to disk in JSON format.

    Parameters
    ----------
    filepath : os.PathLike
        If filepath is a directory, the solution will be stored in a generically named file.
        Optionally, include the filename in the filepath to control the output file name.
    solution : MgmSolution or GmSolution
        The MGM solution to save.
    """
    ...

@typing.overload
def save_to_disk(filepath: os.PathLike, solution: pylibmgm.GmSolution) -> None:
    """Save a GM solution to disk in JSON format.

    Parameters
    ----------
    filepath : os.PathLike
        If filepath is a directory, the solution will be stored in a generically named file.
        Optionally, include the filename in the filepath to control the output file name.
    solution : MgmSolution or GmSolution
        The GM solution to save.
    """
    ...
