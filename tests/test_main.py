import pylibmgm
import pytest

def test_construction_compl(synth_4_model):
    constr = pylibmgm.SequentialGenerator(synth_4_model)
    constr.init(pylibmgm.MgmGenerator.matching_order.random)
    sol = constr.generate()
    assert sol is not None
    assert sol.labeling() is not None
    assert sol.evaluate() < 0
    
    assert all(l >= 0 for gm_labeling in sol.labeling().values() for l in gm_labeling), "Solution is not complete"

@pytest.mark.parametrize("model", ["hotel_4_model", "house_8_model"])
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
