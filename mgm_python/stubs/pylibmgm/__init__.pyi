from __future__ import annotations
import pylibmgm
from pylibmgm import build_sync_problem
import typing

__all__ = ['CostMap', 'GMLocalSearcher', 'GMLocalSearcherParallel', 'GmModel', 'GmSolution', 'Graph', 'LAPSolver', 'MgmGenerator', 'MgmModel', 'MgmSolution', 'ParallelGenerator', 'QAPSolver', 'SequentialGenerator', 'SwapLocalSearcher', 'build_sync_problem', 'omp_set_num_threads']

class CostMap:
    @typing.overload
    def contains(self: pylibmgm.CostMap, arg0: int, arg1: int) -> float:
        ...
    @typing.overload
    def contains(self: pylibmgm.CostMap, arg0: tuple[int, int]) -> float:
        ...
    @typing.overload
    def contains(self: pylibmgm.CostMap, arg0: int, arg1: int, arg2: int, arg3: int) -> float:
        ...
    @typing.overload
    def contains(self: pylibmgm.CostMap, arg0: tuple[tuple[int, int], tuple[int, int]]) -> float:
        ...
    @typing.overload
    def pairwise(self: pylibmgm.CostMap, arg0: int, arg1: int, arg2: int, arg3: int) -> float:
        ...
    @typing.overload
    def pairwise(self: pylibmgm.CostMap, arg0: tuple[tuple[int, int], tuple[int, int]]) -> float:
        ...
    @typing.overload
    def unary(self: pylibmgm.CostMap, arg0: int, arg1: int) -> float:
        ...
    @typing.overload
    def unary(self: pylibmgm.CostMap, arg0: tuple[int, int]) -> float:
        ...
class GMLocalSearcher:
    @typing.overload
    def __init__(self: pylibmgm.GMLocalSearcher, arg0: MgmModel) -> None:
        ...
    @typing.overload
    def __init__(self: pylibmgm.GMLocalSearcher, arg0: MgmModel, arg1: list[int]) -> None:
        ...
    def search(self: pylibmgm.GMLocalSearcher, arg0: MgmSolution) -> bool:
        ...
class GMLocalSearcherParallel:
    def __init__(self: pylibmgm.GMLocalSearcherParallel, model: MgmModel, merge_all: bool = True) -> None:
        ...
    def search(self: pylibmgm.GMLocalSearcherParallel, arg0: MgmSolution) -> bool:
        ...
class GmModel:
    graph1: Graph
    graph2: Graph
    @typing.overload
    def __init__(self: pylibmgm.GmModel, arg0: Graph, arg1: Graph) -> None:
        ...
    @typing.overload
    def __init__(self: pylibmgm.GmModel, arg0: Graph, arg1: Graph, arg2: int, arg3: int) -> None:
        ...
    def add_assignment(self: pylibmgm.GmModel, arg0: int, arg1: int, arg2: float) -> None:
        ...
    @typing.overload
    def add_edge(self: pylibmgm.GmModel, arg0: int, arg1: int, arg2: float) -> None:
        """
        Add an edge via two assignment ids
        """
    @typing.overload
    def add_edge(self: pylibmgm.GmModel, arg0: int, arg1: int, arg2: int, arg3: int, arg4: float) -> None:
        """
        Add an edge via four node ids
        """
    def costs(self: pylibmgm.GmModel) -> CostMap:
        ...
    def no_assignments(self: pylibmgm.GmModel) -> int:
        ...
    def no_edges(self: pylibmgm.GmModel) -> int:
        ...
    @property
    def assignment_list(self) -> list[tuple[int, int]]:
        ...
class GmSolution:
    @staticmethod
    def evaluate_static(arg0: GmModel, arg1: list[int]) -> float:
        ...
    def __getitem__(self: pylibmgm.GmSolution, arg0: int) -> int:
        ...
    @typing.overload
    def __init__(self: pylibmgm.GmSolution) -> None:
        ...
    @typing.overload
    def __init__(self: pylibmgm.GmSolution, arg0: GmModel) -> None:
        ...
    @typing.overload
    def __init__(self: pylibmgm.GmSolution, arg0: GmModel, arg1: list[int]) -> None:
        ...
    def __setitem__(self: pylibmgm.GmSolution, arg0: int, arg1: int) -> None:
        ...
    def evaluate(self: pylibmgm.GmSolution) -> float:
        ...
    def labeling(self: pylibmgm.GmSolution) -> list[int]:
        ...
    def to_list_with_none(self: pylibmgm.GmSolution) -> list:
        ...
class Graph:
    id: int
    no_nodes: int
    def __init__(self: pylibmgm.Graph, arg0: int, arg1: int) -> None:
        ...
class LAPSolver:
    def __init__(self: pylibmgm.LAPSolver, arg0: GmModel) -> None:
        ...
    def run(self: pylibmgm.LAPSolver) -> GmSolution:
        ...
class MgmGenerator:
    class matching_order:
        """
        Members:
        
          sequential
        
          random
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
    random: typing.ClassVar[MgmGenerator.matching_order]  # value = <matching_order.random: 1>
    sequential: typing.ClassVar[MgmGenerator.matching_order]  # value = <matching_order.sequential: 0>
class MgmModel:
    graphs: list[Graph]
    models: dict[tuple[int, int], GmModel]
    no_graphs: int
    def __init__(self: pylibmgm.MgmModel) -> None:
        ...
    def add_model(self: pylibmgm.MgmModel, arg0: GmModel) -> None:
        ...
    def create_submodel(self: pylibmgm.MgmModel, arg0: list[int]) -> pylibmgm.MgmModel:
        ...
class MgmSolution:
    model: MgmModel
    def __getitem__(self: pylibmgm.MgmSolution, arg0: tuple[int, int]) -> list[int]:
        ...
    def __init__(self: pylibmgm.MgmSolution, arg0: MgmModel) -> None:
        ...
    def __len__(self: pylibmgm.MgmSolution) -> int:
        ...
    def __setitem__(self: pylibmgm.MgmSolution, arg0: tuple[int, int], arg1: list[int]) -> None:
        ...
    def create_empty_labeling(self: pylibmgm.MgmSolution) -> dict[tuple[int, int], list[int]]:
        ...
    @typing.overload
    def evaluate(self: pylibmgm.MgmSolution) -> float:
        ...
    @typing.overload
    def evaluate(self: pylibmgm.MgmSolution, arg0: int) -> float:
        ...
    def labeling(self: pylibmgm.MgmSolution) -> dict[tuple[int, int], list[int]]:
        ...
    @typing.overload
    def set_solution(self: pylibmgm.MgmSolution, arg0: dict[tuple[int, int], list[int]]) -> None:
        ...
    @typing.overload
    def set_solution(self: pylibmgm.MgmSolution, arg0: tuple[int, int], arg1: list[int]) -> None:
        ...
    @typing.overload
    def set_solution(self: pylibmgm.MgmSolution, arg0: GmSolution) -> None:
        ...
    def to_dict_with_none(self: pylibmgm.MgmSolution) -> dict:
        ...
class ParallelGenerator(MgmGenerator):
    def __init__(self: pylibmgm.ParallelGenerator, arg0: MgmModel) -> None:
        ...
    def generate(self: pylibmgm.ParallelGenerator) -> MgmSolution:
        ...
    def init(self: pylibmgm.ParallelGenerator, arg0: MgmGenerator.matching_order) -> list[int]:
        ...
class QAPSolver:
    def __init__(self: pylibmgm.QAPSolver, model: GmModel, batch_size: int = 10, greedy_generations: int = 10) -> None:
        ...
    def run(self: pylibmgm.QAPSolver, verbose: bool = False) -> GmSolution:
        ...
class SequentialGenerator(MgmGenerator):
    def __init__(self: pylibmgm.SequentialGenerator, arg0: MgmModel) -> None:
        ...
    def generate(self: pylibmgm.SequentialGenerator) -> MgmSolution:
        ...
    def init(self: pylibmgm.SequentialGenerator, arg0: MgmGenerator.matching_order) -> list[int]:
        ...
    def step(self: pylibmgm.SequentialGenerator) -> None:
        ...
class SwapLocalSearcher:
    def __init__(self: pylibmgm.SwapLocalSearcher, arg0: MgmModel) -> None:
        ...
    def search(self: pylibmgm.SwapLocalSearcher, arg0: MgmSolution) -> bool:
        ...
def omp_set_num_threads(arg0: int) -> None:
    ...