import pylibmgm._pylibmgm as lib

def solve_mgm(model, local_search=True):
    solver = lib.SequentialGenerator(model)
    order = solver.init(lib.SequentialGenerator.matching_order.random)
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
    order = solver.init(lib.SequentialGenerator.matching_order.random)
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

def synchronize_solution(model, solution, feasible=True):
    sync_model = lib.build_sync_problem(model, solution, feasible)

    sync_solution, _ = solve_mgm(sync_model, local_search=True)

    return sync_solution

def solve_gm(gm_model):
    if gm_model.no_edges() == 0:
        solver = lib.LAPSolver(gm_model)
    else:
        solver = lib.QAPSolver(gm_model)

    return solver.run()

def solve_mgm_pairwise(mgm_model):
    # Solve pairwise graph matchings
    solution = lib.MgmSolution(mgm_model)

    indices = sorted(mgm_model.models.keys())
    for gm_idx in indices:
        model = mgm_model.models[gm_idx]
        
        s = solve_gm(model)
        solution[gm_idx] = s

    return solution