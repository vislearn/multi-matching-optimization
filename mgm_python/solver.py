import pylibmgm._pylibmgm as lib

def solve_mgm(model, local_search=True):
    solver = lib.SequentialGenerator(model)
    order = solver.init_generation_sequence(lib.SequentialGenerator.matching_order.random)
    solver.generate()
    if not local_search:
        return solver.export_solution()
    
    cliques = solver.export_cliquemanager()
    local_searcher = lib.LocalSearcher(cliques, order, model)
    local_searcher.search()
    return local_searcher.export_solution()

def solve_gm(model):

    # TODO: This is a temporary solution, as the api has no direct interface to the qap lib as of now.
    qap_solver = lib.QAPSolver(model)
    solution = qap_solver.run()

    return solution