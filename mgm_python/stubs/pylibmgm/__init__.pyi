from __future__ import annotations
import pylibmgm
from pylibmgm import io, solver
import typing

__all__ = ['CostMap', 'GMLocalSearcher', 'GMLocalSearcherParallel', 'GmModel', 'GmSolution', 'Graph', 'LAPSolver', 'MgmGenerator', 'MgmModel', 'MgmSolution', 'ParallelGenerator', 'QAPSolver', 'SequentialGenerator', 'SwapLocalSearcher', 'build_sync_problem', 'omp_set_num_threads', 'io', 'solver']

class CostMap:
    """Container for unary and pairwise costs in a graph matching problem.
    
    Stores assignment costs (unary) and edge costs (pairwise) for a GM model.
    Provides efficient lookup of costs using node indices or assignment/edge structures.

    Only exposed for lookup. Setting of costs is handled by GmModel class.

    Uses a ankerl::unordered_dense::map HashMap in the background. Especially designed for very sparse problems. 
    Not ideal for dense, and not as sparse problems could potentially benefit from a sparse matrix instead.
    """
    @typing.overload
    def contains(self: pylibmgm.CostMap, node1: int, node2: int) -> bool:
        """Check if unary cost exists for given node pair."""
        ...
    @typing.overload
    def contains(self: pylibmgm.CostMap, assignment: tuple[int, int]) -> bool:
        """Check if unary cost exists for given (node1, node2) assignment tuple."""
        ...
    @typing.overload
    def contains(self: pylibmgm.CostMap, assignment1_left: int, assignment1_right: int, assignment2_left: int, assignment2_right: int) -> bool:
        """Check if pairwise cost exists for given node quadruple."""
        ...
    @typing.overload
    def contains(self: pylibmgm.CostMap, edge: tuple[tuple[int, int], tuple[int, int]]) -> bool:
        """Check if pairwise cost exists for given edge ((assignment1_left, assignment1_right), (assignment2_left, assignment2_right)) tuple."""
        ...
    @typing.overload
    def pairwise(self: pylibmgm.CostMap, assignment1_left: int, assignment1_right: int, assignment2_left: int, assignment2_right: int) -> float:
        """Get pairwise cost for edge defined by a node quadruple."""
        ...
    @typing.overload
    def pairwise(self: pylibmgm.CostMap, edge: tuple[tuple[int, int], tuple[int, int]]) -> float:
        """Get pairwise cost for an edge index."""
        ...
    @typing.overload
    def unary(self: pylibmgm.CostMap, node1: int, node2: int) -> float:
        """Get unary cost for matching node1 (from graph1) to node2 (from graph2)."""
        ...
    @typing.overload
    def unary(self: pylibmgm.CostMap, assignment: tuple[int, int]) -> float:
        """Get unary cost for an assignment index."""
        ...
        
class GMLocalSearcher:
    """Local search method based on iteratively re-matching graphs.
    
    Improves a solution by re-solving pairwise graph matching problems
    while keeping other matchings fixed.
    """
    @typing.overload
    def __init__(self: pylibmgm.GMLocalSearcher, model: MgmModel) -> None:
        """Initialize with an MGM model.
        
        Parameters
        ----------
        model : MgmModel
            The MGM model.
        """
        ...
    @typing.overload
    def __init__(self: pylibmgm.GMLocalSearcher, model: MgmModel, matching_order: list[int]) -> None:
        """Initialize with an MGM model and search order.
        
        Parameters
        ----------
        model : MgmModel
            The MGM model.
        matching_order : list[int]
            Order in which to re-match graphs.
        """
        ...
    def search(self: pylibmgm.GMLocalSearcher, solution: MgmSolution) -> bool:
        """Perform local search to improve the solution.
        
        Parameters
        ----------
        solution : MgmSolution
            Solution to improve (modified in-place).
            
        Returns
        -------
        bool
            True if solution was improved.
        """
        ...
        
class GMLocalSearcherParallel:
    """Parallel version of GM local search.
    
    Similar to GMLocalSearcher but tries the re-matche for all graphs in parallel.
    Ranking the results by improvement,
    it applies the best improvement and stops if merge_all = False.
    If merge_all = True, it further checks all other improving re-matches iteratively
    and merges them as well, if they still improve the solution after.
    Provides faster optimization. 
    """
    def __init__(self: pylibmgm.GMLocalSearcherParallel, model: MgmModel, merge_all: bool = True) -> None:
        """Initialize parallel GM local searcher.
        
        Parameters
        ----------
        model : MgmModel
            The MGM model.
        merge_all : bool, optional
            Whether to merge all parallel results at once (default: True).
        """
        ...
    def search(self: pylibmgm.GMLocalSearcherParallel, solution: MgmSolution) -> bool:
        """Perform parallel local search to improve the solution.
        
        Parameters
        ----------
        solution : MgmSolution
            Solution to improve (modified in-place).
            
        Returns
        -------
        bool
            True if solution was improved.
        """
        ...
        
class GmModel:
    """Pairwise graph matching (GM) model.
    
    Represents a matching problem between two graphs with unary assignment costs
    and pairwise edge costs. This is the core data structure for single pairwise
    graph matching problems.
    """
    graph1: Graph
    """First graph in the matching."""
    
    graph2: Graph
    """Second graph in the matching."""
    
    @typing.overload
    def __init__(self: pylibmgm.GmModel, graph1: Graph, graph2: Graph) -> None:
        """Create a GM model between two graphs.
        
        Parameters
        ----------
        graph1 : Graph
            First graph to match.
        graph2 : Graph
            Second graph to match.
        """
        ...
    @typing.overload
    def __init__(self: pylibmgm.GmModel, graph1: Graph, graph2: Graph, no_assignments: int, no_edges: int) -> None:
        """Create a GM model with pre-allocation.
        
        Parameters
        ----------
        graph1 : Graph
            First graph to match.
        graph2 : Graph
            Second graph to match.
        no_assignments : int
            Expected number of assignments (for pre-allocation).
        no_edges : int
            Expected number of edges (for pre-allocation).
        """
        ...
    def add_assignment(self: pylibmgm.GmModel, node1: int, node2: int, cost: float) -> None:
        """Add an assignment with its unary cost.
        
        Parameters
        ----------
        node1 : int
            Node from graph1.
        node2 : int
            Node from graph2.
        cost : float
            Unary cost of this assignment.
        """
        ...
    @typing.overload
    def add_edge(self: pylibmgm.GmModel, assignment1: int, assignment2: int, cost: float) -> None:
        """Add an edge via two assignment IDs.
        
        Parameters
        ----------
        assignment1 : int
            First assignment ID.
        assignment2 : int
            Second assignment ID.
        cost : float
            Pairwise cost of this edge.
        """
    @typing.overload
    def add_edge(self: pylibmgm.GmModel, assignment1_left: int, assignment1_right: int, assignment2_left: int, assignment2_right: int, cost: float) -> None:
        """Add an edge via four node IDs.
        
        Parameters
        ----------
        assignment1_left : int
            First node of first assignment.
        assignment1_right : int
            Second node of first assignment.
        assignment2_left : int
            First node of second assignment.
        assignment2_right : int
            Second node of second assignment.
        cost : float
            Pairwise cost of this edge.
        """
    def costs(self: pylibmgm.GmModel) -> CostMap:
        """Access the cost hashmap.
        
        Returns
        -------
        CostMap
            The cost hashmap containing all unary and pairwise costs.
        """
        ...
    def no_assignments(self: pylibmgm.GmModel) -> int:
        """Get number of assignments.
        
        Returns
        -------
        int
            Number of assignments in the model.
        """
        ...
    def no_edges(self: pylibmgm.GmModel) -> int:
        """Get number of edges.
        
        Returns
        -------
        int
            Number of edges in the model.
        """
        ...
    @property
    def assignment_list(self) -> list[tuple[int, int]]:
        """List of all assignments. Index is assignment ID.
        
        Returns
        -------
        list[tuple[int, int]]
            List of (node1, node2) assignment tuples.
        """
        ...
        
class GmSolution:
    """Solution to a pairwise graph matching problem.
    
    Represents an assignment of nodes from graph1 to nodes in graph2 (or unmatched).
    The labeling uses -1 to indicate that a node is unmatched.
    """
    model: GmModel
    """The GM model for this solution."""
    
    @staticmethod
    def evaluate_static(model: GmModel, labeling: list[int]) -> float:
        """Evaluate objective value for a given model and labeling.
        
        Parameters
        ----------
        model : GmModel
            Graph matching model.
        labeling : list[int]
            Node assignments.
            
        Returns
        -------
        float
            Total objective value (sum of unary and pairwise costs).
        """
        ...
    def __getitem__(self: pylibmgm.GmSolution, node_id: int) -> int:
        """Get assignment for a node."""
        ...
    @typing.overload
    def __init__(self: pylibmgm.GmSolution) -> None:
        """Create an empty GM solution."""
        ...
    @typing.overload
    def __init__(self: pylibmgm.GmSolution, model: GmModel) -> None:
        """Create a GM solution for a model.
        
        Parameters
        ----------
        model : GmModel
            The GM model this solution belongs to.
        """
        ...
    @typing.overload
    def __init__(self: pylibmgm.GmSolution, model: GmModel, labeling: list[int]) -> None:
        """Create a GM solution with initial labeling.
        
        Parameters
        ----------
        model : GmModel
            The GM model this solution belongs to.
        labeling : list[int]
            Initial labeling. For each node i in graph1, labeling[i] is the matched
            node in graph2, or -1 if unmatched.
        """
        ...
    def __setitem__(self: pylibmgm.GmSolution, node_id: int, label: int) -> None:
        """Set assignment for a node."""
        ...
    def evaluate(self: pylibmgm.GmSolution) -> float:
        """Evaluate objective value of this solution.
        
        Returns
        -------
        float
            Total objective value.
        """
        ...
    def labeling(self: pylibmgm.GmSolution) -> list[int]:
        """Get the labeling as a list.

        Index indicates node ID of graph1. Value is node ID of graph2.
        Value of -1 indicates an unmatched node.
        
        Returns
        -------
        list[int]
            Node assignments.
        """
        ...
    def to_list_with_none(self: pylibmgm.GmSolution) -> list:
        """Convert labeling to Python list with None for unmatched nodes.
        
        Returns
        -------
        list
            Labeling where -1 is replaced with None.
        """
        ...
        
class Graph:
    """Represents a single graph in a multi-graph matching problem.
    
    A graph is defined by an ID and the number of nodes it contains.
    """
    id: int
    """Unique graph identifier."""
    
    no_nodes: int
    """Number of nodes in the graph."""
    
    def __init__(self: pylibmgm.Graph, graph_id: int, no_nodes: int) -> None:
        """Create a graph.
        
        Parameters
        ----------
        graph_id : int
            Unique identifier for the graph.
        no_nodes : int
            Number of nodes in the graph.
        """
        ...
        
class LAPSolver:
    """Linear Assignment Problem solver.
    
    Solves pairwise graph matching problems with only unary costs (no pairwise terms) to optimality.
    Calls the implementation of Scipy's linear_sum_assignment function.
    """
    def __init__(self: pylibmgm.LAPSolver, model: GmModel) -> None:
        """Initialize LAP solver.
        
        Parameters
        ----------
        model : GmModel
            The graph matching model (should have no edges, only assignments).
        """
        ...
    def run(self: pylibmgm.LAPSolver) -> GmSolution:
        """Solve the linear assignment problem.
        
        Returns
        -------
        GmSolution
            Optimal solution.
        """
        ...
        
class MgmGenerator:
    """Abstract base class for multi-graph matching solution generators."""
    class matching_order:
        """Order in which graphs are matched during solution generation.
        
        Attributes
        ----------
        sequential
            Match graphs in sequential order (0, 1, 2, ...).
        random
            Match graphs in random order.
        """
        __members__: typing.ClassVar[dict[str, MgmGenerator.matching_order]]  # value = {'sequential': <matching_order.sequential: 0>, 'random': <matching_order.random: 1>}
        random: typing.ClassVar[MgmGenerator.matching_order]  # value = <matching_order.random: 1>
        sequential: typing.ClassVar[MgmGenerator.matching_order]  # value = <matching_order.sequential: 0>
        def __eq__(self, other: typing.Any) -> bool:
            ...
        def __getstate__(self) -> int:
            ...
        def __hash__(self) -> int:
            ...
        def __index__(self) -> int:
            ...
        def __init__(self, value: int) -> None:
            ...
        def __int__(self) -> int:
            ...
        def __ne__(self, other: typing.Any) -> bool:
            ...
        def __repr__(self) -> str:
            ...
        def __setstate__(self, state: int) -> None:
            ...
        def __str__(self) -> str:
            ...
        @property
        def name(self) -> str:
            ...
        @property
        def value(self) -> int:
            ...
    
class MgmModel:
    """Multi-graph matching (MGM) model.
    
    Represents a collection of graphs and pairwise GM models between them.
    This is the main data structure for multi-graph matching problems.
    """
    graphs: list[Graph]
    """List of all graphs.
    
    IDs graph.id should be sorted in ascending order starting from zero: (0,1,2,3,4,...).
    """
    
    models: dict[tuple[int, int], GmModel]
    """Dictionary of pairwise GM models indexed by (graph1_id, graph2_id)."""
    
    no_graphs: int
    """Number of graphs in the problem."""
    
    def __init__(self: pylibmgm.MgmModel) -> None:
        """Create an empty MGM model."""
        ...
    def add_model(self: pylibmgm.MgmModel, gm_model: GmModel) -> None:
        """Add a pairwise GM model to this MGM problem.
        
        Parameters
        ----------
        gm_model : GmModel
            Pairwise model to add.
        """
        ...
    def create_submodel(self: pylibmgm.MgmModel, graph_ids: list[int]) -> pylibmgm.MgmModel:
        """Create a submodel containing only the specified graphs.

        Note: Graph ids of returned problem will have IDs (0,1,2,3,...)
        and thus may not coincide with given problems graph IDs.
        
        Parameters
        ----------
        graph_ids : list[int]
            IDs of graphs to include in the submodel.
            
        Returns
        -------
        MgmModel
            New model with only the specified graphs and their pairwise models.
        """
        ...
        
class MgmSolution:
    """Solution to a multi-graph matching problem.
    
    Represents a consistent set of pairwise matchings across multiple graphs.
    The solution can be inspected as a labeling (given for all pairwise models) or as 
    the cycle-consistent set of cliques.
    """
    model: MgmModel
    """The MGM model for this solution."""
    
    def __getitem__(self: pylibmgm.MgmSolution, graph_pair: tuple[int, int]) -> list[int]:
        """Get labeling for a specific pairwise model by (graph1_id, graph2_id)."""
        ...
    def __init__(self: pylibmgm.MgmSolution, model: MgmModel) -> None:
        """Create an MGM solution.
        
        Parameters
        ----------
        model : MgmModel
            The MGM model this solution belongs to.
        """
        ...
    def __len__(self: pylibmgm.MgmSolution) -> int:
        """Get number of pairwise solutions."""
        ...
    def __setitem__(self: pylibmgm.MgmSolution, graph_pair: tuple[int, int], labeling: list[int]) -> None:
        """Set labeling for a specific pairwise model."""
        ...
    def cliques(self: pylibmgm.MgmSolution) -> list[dict]:
        """Get the clique structure of the solution.
        
        Returns
        -------
        list[dict]
            List of cliques, each representing matched nodes across graphs.
        """
        ...
    def create_empty_labeling(self: pylibmgm.MgmSolution) -> dict[tuple[int, int], list[int]]:
        """Create an empty labeling structure fitting the solutions underlying model.
        
        Returns
        -------
        dict[tuple[int, int], list[int]]
            Empty labeling dictionary with correct structure.
        """
        ...
    @typing.overload
    def evaluate(self: pylibmgm.MgmSolution) -> float:
        """Evaluate total objective value across all pairwise models.
        
        Returns
        -------
        float
            Total objective value.
        """
        ...
    @typing.overload
    def evaluate(self: pylibmgm.MgmSolution, graph_id: int) -> float:
        """Evaluate objective restricted to models involving a specific graph.
        
        Parameters
        ----------
        graph_id : int
            Graph ID to evaluate.
            
        Returns
        -------
        float
            Objective value for models involving this graph.
        """
        ...
    def labeling(self: pylibmgm.MgmSolution) -> dict[tuple[int, int], list[int]]:
        """Get the full labeling as a dictionary.
        
        Returns
        -------
        dict[tuple[int, int], list[int]]
            Dictionary mapping (graph1_id, graph2_id) to list of assignments.
        """
        ...
    @typing.overload
    def set_solution(self: pylibmgm.MgmSolution, labeling: dict[tuple[int, int], list[int]]) -> None:
        """Set solution from a labeling dictionary."""
        ...
    @typing.overload
    def set_solution(self: pylibmgm.MgmSolution, graph_pair: tuple[int, int], labeling: list[int]) -> None:
        """Set solution for a specific pairwise model."""
        ...
    @typing.overload
    def set_solution(self: pylibmgm.MgmSolution, gm_solution: GmSolution) -> None:
        """Update solution with a pairwise GM solution."""
        ...
    def to_dict_with_none(self: pylibmgm.MgmSolution) -> dict:
        """Convert labeling to dictionary with None for unmatched nodes.
        
        Returns
        -------
        dict
            Labeling dictionary where -1 is replaced with None.
        """
        ...
        
class ParallelGenerator(MgmGenerator):
    """Parallel solution generator for multi-graph matching.
    
    Constructs a solution by matching graphs in parallel in a tree structure,
    starting with pairs of graphs as the leaves.
    Can be faster for large problems, may produce different
    results than sequential generation. Showed better results in experiments than sequential generator.
    """
    def __init__(self: pylibmgm.ParallelGenerator, model: MgmModel) -> None:
        """Initialize parallel generator.
        
        Parameters
        ----------
        model : MgmModel
            The MGM model to solve.
        """
        ...
    def generate(self: pylibmgm.ParallelGenerator) -> MgmSolution:
        """Generate a complete solution using parallel matching.
        
        Returns
        -------
        MgmSolution
            Generated cycle-consistent solution.
        """
        ...
    def init(self: pylibmgm.ParallelGenerator, order: MgmGenerator.matching_order) -> list[int]:
        """Initialize the generator with a matching order.
        
        Parameters
        ----------
        order : MgmGenerator.matching_order
            Order to match graphs (sequential or random).
            
        Returns
        -------
        list[int]
            Sequence of graph IDs to be matched.
        """
        ...
        
class QAPSolver:
    """Quadratic Assignment Problem solver for graph matching.
    
    Solves pairwise graph matching problems using the Fusion Moves solver [1]_.
    
    References
    ----------
    .. [1] Fusion Moves for Graph Matching, Lisa Hutschenreiter et al., ICCV 2021.
    .. [2] Tomas Dlask, Bogdan Savchynskyy, Relative-Interior Solution for (Incomplete) Linear Assignment Problem with Applications to Quadratic Assignment Problem. Annals of Mathematics and Artificial Intelligence, 2025
    """
    class RunSettings:
        """Configuration for the QAP solver run parameters."""
        batch_size: int
        """Number of steps per batch (default: 10)."""
        
        greedy_generations: int
        """Number of greedy generation per step (default: 10)."""
        
        def __init__(self: pylibmgm.QAPSolver.RunSettings) -> None:
            ...
    class StoppingCriteria:
        """Stopping criteria for the QAP solver. 
        
        Set a hard upper limit with max_batches.
        Set a relative improvement limit using p and k. Compares upper first, middle and last upperbound (ub) obtained w.r.t. batch iteration.
        Optimization is stopped, if for k consecutive iterations it holds: \n\t ``abs(last_ub - middle_ub) <= p * abs(middle_ub - first_ub)).``

        See Sec 5.3 of [2]_ for details.
        """
        p: int
        """Convergence threshold parameter (default: 0.6)."""
        
        k: int
        """Number of consecutive iterations the convergence criteria has to be fulfilled."""
        
        max_batches: int
        """Maximum number of batches to run."""
        
        def __init__(self: pylibmgm.QAPSolver.StoppingCriteria) -> None:
            ...
    run_settings: RunSettings
    """Run configuration settings."""
    
    stopping_criteria: StoppingCriteria
    """Stopping criteria for the solver."""
    
    default_run_settings: typing.ClassVar[RunSettings]
    """Default run settings. Modify to globally affect new instances and MGM algorithms."""
    
    default_stopping_criteria: typing.ClassVar[StoppingCriteria]
    """Default stopping criteria. Modify to globally affect new instances and MGM algorithms."""
    
    libmpopt_seed: typing.ClassVar[int]
    """Random seed for the Fusion Moves solver."""
    
    def __init__(self, model: GmModel) -> None:
        """Initialize QAP solver.
        
        Parameters
        ----------
        model : GmModel
            The pairwise graph matching model to solve.
        """
        ...
    def run(self, verbose: bool = False) -> GmSolution:
        """Run the QAP solver.
        
        Parameters
        ----------
        verbose : bool, optional
            Enable verbose output (default: False).
            
        Returns
        -------
        GmSolution
            Optimized solution.
        """
        ...
        
class SequentialGenerator(MgmGenerator):
    """Sequential solution generator for multi-graph matching.
    
    Constructs a solution by sequentially matching graphs one at a time,
    maintaining cycle consistency at each step.
    """
    def __init__(self: pylibmgm.SequentialGenerator, model: MgmModel) -> None:
        """Initialize sequential generator.
        
        Parameters
        ----------
        model : MgmModel
            The MGM model to solve.
        """
        ...
    def generate(self: pylibmgm.SequentialGenerator) -> MgmSolution:
        """Generate a complete solution.
        
        Returns
        -------
        MgmSolution
            Generated cycle-consistent solution.
        """
        ...
    def init(self: pylibmgm.SequentialGenerator, order: MgmGenerator.matching_order) -> list[int]:
        """Initialize the generator with a matching order.
        
        Parameters
        ----------
        order : MgmGenerator.matching_order
            Order to match graphs (sequential or random).
            
        Returns
        -------
        list[int]
            Sequence of graph IDs to be matched.
        """
        ...
    def step(self: pylibmgm.SequentialGenerator) -> None:
        """Perform a single step of the generation process. Match the next graph in the matching order."""
        ...
        
class SwapLocalSearcher:
    """Swap-based local search for multi-graph matching.
    
    Improves the solution by swapping node assignments between pairs of cliques.
    Complements GMLocalSearcher.
    """
    def __init__(self: pylibmgm.SwapLocalSearcher, model: MgmModel) -> None:
        """Initialize swap local searcher.
        
        Parameters
        ----------
        model : MgmModel
            The MGM model.
        """
        ...
    def search(self: pylibmgm.SwapLocalSearcher, solution: MgmSolution) -> bool:
        """Perform swap-based local search.
        
        Parameters
        ----------
        solution : MgmSolution
            Solution to improve (modified in-place).
            
        Returns
        -------
        bool
            True if solution was improved.
        """
        ...
        
def build_sync_problem(model: MgmModel, solution: MgmSolution, feasible: bool = True) -> MgmModel:
    """Build a synchronization problem from an MGM model and solution.
    
    Constructs a new MGM model where the objective is to find a cycle-consistent
    solution close to the given (inconsistent) solution.
    
    Parameters
    ----------
    model : MgmModel
        Original MGM model.
    solution : MgmSolution
        Initial solution (cycle-inconsistent).
    feasible : bool, optional
        If True, only consider matchings present in the original model (default: True).
        
    Returns
    -------
    MgmModel
        Synchronization problem as an MGM model.
    """
    ...
    
def omp_set_num_threads(num_threads: int) -> None:
    """Set the number of OpenMP threads for parallel computation.
    
    Parameters
    ----------
    num_threads : int
        Number of threads to use.
    """
    ...