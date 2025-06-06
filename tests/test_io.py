import pylibmgm
import pytest

from math import isclose

import pathlib
from pathlib import Path

def test_parsing(synth_4_model):
    assert synth_4_model is not None
    assert synth_4_model.no_graphs == 4
    assert len(synth_4_model.graphs) == 4
    assert synth_4_model.models[(0,1)].costs().unary(0,0) == -1.0001
    assert synth_4_model.models[(0,1)].costs().unary(0,1) == -1.0001
    assert synth_4_model.models[(0,1)].costs().unary(0,2) == -1.0001
    assert synth_4_model.models[(0,1)].costs().unary(9,9) == -1.0001
    assert synth_4_model.models[(0,1)].costs().pairwise(0, 0, 1, 1) == -1.7544
    assert synth_4_model.models[(0,1)].costs().pairwise(1, 0, 5, 3) == -1.9221
    assert synth_4_model.models[(0,1)].costs().pairwise(9, 8, 8, 9) == -1.3398

def test_solution_storing_loading(hotel_4_model, tmp_path):
        outpath = tmp_path / "sol.json"

        sol = pylibmgm.solver.solve_mgm(hotel_4_model, pylibmgm.solver.OptimizationLevel.FAST)

        pylibmgm.io.save_to_disk(outpath, sol)
        sol_parsed = pylibmgm.io.import_solution(outpath, hotel_4_model)

        assert(sol.labeling() == sol_parsed.labeling())
        assert(isclose(sol.evaluate(), sol_parsed.evaluate()))

class TestSafeToDiskFilename:  
    def test_default(self, hotel_4_model, tmp_path):
        outpath = tmp_path / "hotel_4_solution_1.json"
        outpath_expected = tmp_path / "hotel_4_solution_1.json"

        sol = pylibmgm.solver.solve_mgm(hotel_4_model, pylibmgm.solver.OptimizationLevel.FAST)

        pylibmgm.io.save_to_disk(outpath, sol)

        assert(outpath.exists())
        assert(outpath == outpath_expected)    
        
    def test_missing_extension(self, hotel_4_model, tmp_path):
        outpath = tmp_path / "hotel_4_solution_1"
        outpath_expected = tmp_path / "hotel_4_solution_1.json"

        sol = pylibmgm.solver.solve_mgm(hotel_4_model, pylibmgm.solver.OptimizationLevel.FAST)

        pylibmgm.io.save_to_disk(outpath, sol)

        assert(outpath_expected.exists())

    def test_wrong_extension(self, hotel_4_model, tmp_path):
        outpath = tmp_path / "hotel_4_solution_1.png"
        outpath_expected = tmp_path / "hotel_4_solution_1.json"

        sol = pylibmgm.solver.solve_mgm(hotel_4_model, pylibmgm.solver.OptimizationLevel.FAST)

        pylibmgm.io.save_to_disk(outpath, sol)

        assert(outpath_expected.exists())

    def test_outpath_is_folder(self, hotel_4_model, tmp_path):
        outpath = tmp_path
        outpath_expected = tmp_path / "solution.json"

        sol = pylibmgm.solver.solve_mgm(hotel_4_model, pylibmgm.solver.OptimizationLevel.FAST)

        pylibmgm.io.save_to_disk(outpath, sol)

        assert(outpath_expected.exists())