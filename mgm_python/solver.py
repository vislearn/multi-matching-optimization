import pylibmgm._pylibmgm as lib

def solve_mgm(model, local_search=True):
    solver = lib.SequentialGenerator(model)
    order = solver.init_generation_sequence(lib.SequentialGenerator.matching_order.random)
    solver.generate()

    cm = solver.export_cliquemanager()

    if not local_search:
        no_cliques = cm.cliques.no_cliques
        return solver.export_solution(), no_cliques
    
    local_searcher = lib.LocalSearcher(cm, order, model)
    local_searcher.search()

    cm = local_searcher.export_cliquemanager()
    return local_searcher.export_solution(), cm.cliques.no_cliques

def solve_mgm_swap(model, iterations=3):
    #generate
    solver = lib.SequentialGenerator(model)
    order = solver.init_generation_sequence(lib.SequentialGenerator.matching_order.random)
    solver.generate()
    
    #Gm local search
    gm_cliques = solver.export_cliquemanager()

    for i in range (iterations):
        print(f"Iteration {i+1}" )
        gm_searcher = lib.LocalSearcher(gm_cliques, order, model)
        gm_searcher.search()

        swap_cliques = gm_searcher.export_cliquetable()

        swap_searcher = lib.ABOptimizer(swap_cliques, model)
        swap_searcher.search()
        swap_cliques = swap_searcher.export_cliquetable()

        gm_cliques.reconstruct_from(swap_cliques)

    print(f"Final number of cliques: {gm_cliques.cliques.no_cliques}")
    return swap_searcher.export_solution()

def solve_gm(model):

    # TODO: This is a temporary solution, as the api has no direct interface to the qap lib as of now.
    qap_solver = lib.QAPSolver(model)
    solution = qap_solver.run()

    return solution