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

def test_qap_solver_verbose_mode(opengm_model, caplog):
    """Test that QAPSolver with verbose=True logs stopping criterion message to spdlog."""
    import logging
    
    # Set logging level to capture info messages from the libmgm logger
    caplog.set_level(logging.INFO, logger="libmgm")
    
    solver = pylibmgm.QAPSolver(opengm_model)
    
    # Run with verbose=True
    sol = solver.run(verbose=True)
    
    # Check that solution is valid
    assert sol is not None
    assert len(sol.labeling()) > 0
    assert sol.evaluate() < 0
    
    # Check that the stopping criterion message was logged
    # The message should contain "Stopping criterion met in iteration"
    log_messages = [record.message for record in caplog.records if record.name == "libmgm"]

   
    assert any("lb=" in msg for msg in log_messages), "Missing print of lower bound in verbose QAPSolver mode"
    assert any("ub=" in msg for msg in log_messages), "Missing print of upper bound in verbose QAPSolver mode"
    assert any("gap=" in msg for msg in log_messages), "Missing print of gap in verbose QAPSolver mode"
    assert any("t=" in msg for msg in log_messages), "Missing print of time taken in verbose QAPSolver mode"
    assert any("a=" in msg for msg in log_messages), "Missing print of intermediate assignment in verbose QAPSolver mode"

class TestExportDDFile:
    def test_creates_parent_directory(self, hotel_4_model, tmp_path):
        """Test that export_dd_file creates parent directories if they don't exist."""
        outpath = tmp_path / "new_folder" / "model.dd"
        
        pylibmgm.io.export_dd_file(outpath, hotel_4_model)
        
        assert outpath.exists()
        assert outpath.is_file()
    
    def test_directory_path_uses_default_filename(self, hotel_4_model, tmp_path):
        """Test that passing a directory path uses the default filename 'mgm_model.dd'."""
        outpath = tmp_path / "output_dir"
        outpath.mkdir()
        outpath_expected = outpath / "mgm_model.dd"
        
        pylibmgm.io.export_dd_file(outpath, hotel_4_model)
        
        assert outpath_expected.exists()