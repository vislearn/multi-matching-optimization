import pylibmgm
import pytest

from pathlib import Path

# Optimum is complete matching
synthetic_complete_4 = "synthetic_complete_instance_1_nNodes_10_nGraphs_4.txt"

# Optimum is incomplete matching
hotel_4 = "hotel_instance_1_nNodes_10_nGraphs_4.txt"
house_8 = "house_instance_1_nNodes_10_nGraphs_8.txt"

# Sparse problem
worms_3 = "worms_3_small.dd"

# GM problem
opengm = "opengm1.dd"

@pytest.fixture(scope="session")
def synth_4_model():
    model_path = Path(__file__).parent / synthetic_complete_4
    model = pylibmgm.io.parse_dd_file(model_path)
    return model

@pytest.fixture(scope="session")
def hotel_4_model():
    model_path = Path(__file__).parent / hotel_4
    model = pylibmgm.io.parse_dd_file(model_path)
    return model

@pytest.fixture(scope="session") 
def house_8_model():
    model_path = Path(__file__).parent / house_8
    model = pylibmgm.io.parse_dd_file(model_path)
    return model

@pytest.fixture(scope="session") 
def worms_3_model():
    model_path = Path(__file__).parent / worms_3
    model = pylibmgm.io.parse_dd_file(model_path)
    return model

@pytest.fixture(scope="session") 
def opengm_model():
    model_path = Path(__file__).parent / opengm
    model = pylibmgm.io.parse_dd_file_gm(model_path)
    return model

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

def test_construction_compl(synth_4_model):
    constr = pylibmgm.SequentialGenerator(synth_4_model)
    constr.init(pylibmgm.MgmGenerator.matching_order.random)
    sol = constr.generate()
    assert sol is not None
    assert sol.labeling() is not None
    assert sol.evaluate() < 0
    
    assert all(l >= 0 for gm_labeling in sol.labeling().values() for l in gm_labeling), "Solution is not complete"

@pytest.mark.parametrize("model", ["hotel_4_model", "house_8_model", "worms_3_model"])
def test_construction_incompl(request, model):
    m = request.getfixturevalue(model)
    constr = pylibmgm.SequentialGenerator(m)
    constr.init(pylibmgm.MgmGenerator.matching_order.random)
    sol = constr.generate()
    assert sol is not None
    assert sol.labeling() is not None
    assert sol.evaluate() < 0
    
    assert not all(l >= 0 for gm_labeling in sol.labeling().values() for l in gm_labeling), "Solution should be incomplete"

def test_gm_solver(opengm_model):
    solver = pylibmgm.QAPSolver(opengm_model)
    sol = solver.run()
    assert sol is not None
    assert sol.labeling() is not None
    assert sol.evaluate() < 0

    assert all(l >= 0 for l in sol.labeling()), "Solution is not complete"

def test_LAP_solver(opengm_model):
    solver = pylibmgm.LAPSolver(opengm_model)
    sol = solver.run()
    assert sol is not None
    assert sol.labeling() is not None
    assert sol.evaluate() < 0

    assert all(l >= 0 for l in sol.labeling()), "Solution is not complete"

def test_QAP_better_LAP(opengm_model):
    solver = pylibmgm.LAPSolver(opengm_model)
    sol_LAP = solver.run()

    solver = pylibmgm.QAPSolver(opengm_model)
    sol_QAP = solver.run()

    assert sol_LAP.evaluate() > sol_QAP.evaluate(), "QAP solution should be better than LAP solution"

@pytest.mark.parametrize("model", ["hotel_4_model", "house_8_model", "worms_3_model"])
def test_construction_incompl(request, model):
    m = request.getfixturevalue(model)

@pytest.mark.parametrize("model", ["hotel_4_model", "house_8_model", "synth_4_model"])
@pytest.mark.parametrize("opt_mode", [pylibmgm.solver.OptimizationLevel.FAST,
                                      pylibmgm.solver.OptimizationLevel.DEFAULT,
                                      pylibmgm.solver.OptimizationLevel.EXHAUSTIVE])
def test_run_modes(request, model, opt_mode):
    m = request.getfixturevalue(model)
    sol = pylibmgm.solver.solve_mgm(m, opt_mode)
    assert sol is not None
    assert sol.labeling() is not None
    assert sol.evaluate() < 0

    for (g1, g2), labeling in sol.labeling().items():
        assert all(-1 <= l < m.graphs[g2].no_nodes for l in labeling), "Invalid label in solution."

@pytest.mark.parametrize("model", ["hotel_4_model", "house_8_model", "synth_4_model"])
@pytest.mark.parametrize("opt_mode", [pylibmgm.solver.OptimizationLevel.FAST,
                                      pylibmgm.solver.OptimizationLevel.DEFAULT,
                                      pylibmgm.solver.OptimizationLevel.EXHAUSTIVE])

def test_run_modes_parallel(request, model, opt_mode):    
    m = request.getfixturevalue(model)
    sol = pylibmgm.solver.solve_mgm_parallel(m, opt_mode)
    assert sol is not None
    assert sol.labeling() is not None
    assert sol.evaluate() < 0

    for (g1, g2), labeling in sol.labeling().items():
        assert all(-1 <= l < m.graphs[g2].no_nodes for l in labeling), "Invalid label in solution."