pylibmgm.GmModel
================

.. currentmodule:: pylibmgm



.. autoclass:: pylibmgm.GmModel
   :members:
   :member-order: bysource
   :exclude-members: __delattr__, __dir__, __eq__, __format__, __ge__, __getattribute__, __getstate__, __gt__, __hash__, __init_subclass__, __le__, __lt__, __ne__, __new__, __reduce__, __reduce_ex__, __repr__, __setattr__, __sizeof__, __str__, __subclasshook__, __weakref__, __dict__, __annotations__, __doc__, __module__, assignment_list, graph1, graph2, __init__, add_edge

   
   
   
   .. rubric:: Methods

   .. autosummary::
   
      ~GmModel.__init__
      ~GmModel.add_assignment
      ~GmModel.add_edge
      ~GmModel.costs
      ~GmModel.no_assignments
      ~GmModel.no_edges
   
   

   
   
   
   .. rubric:: Attributes

   .. list-table::
      :widths: 25 20 55
      :header-rows: 1

      * - Name
        - Type
        - Description
      
      * - **assignment_list**
        - list[tuple[int, int]]

        - List of all assignments.
      * - **graph1**
        - :class:`~pylibmgm.Graph`

        - First graph in the matching.
      * - **graph2**
        - :class:`~pylibmgm.Graph`

        - Second graph in the matching.
   
   

   
   
   
   
   
   

   
   
   
   

   
   .. py:method:: __init__(graph1: Graph, graph2: Graph) -> None
      :noindex:

      
      Create a GM model between two graphs.

      :param graph1: First graph to match.
      :type graph1: Graph
      :param graph2: Second graph to match.
      :type graph2: Graph

      

   
   .. py:method:: __init__(graph1: Graph, graph2: Graph, no_assignments: int, no_edges: int) -> None
      :noindex:

      
      Create a GM model with pre-allocation.

      :param graph1: First graph to match.
      :type graph1: Graph
      :param graph2: Second graph to match.
      :type graph2: Graph
      :param no_assignments: Expected number of assignments (for pre-allocation).
      :type no_assignments: int
      :param no_edges: Expected number of edges (for pre-allocation).
      :type no_edges: int

      

   

   
   .. py:method:: add_edge(assignment1: int, assignment2: int, cost: float) -> None
      :noindex:

      
      Add an edge via two assignment IDs.

      :param assignment1: First assignment ID.
      :type assignment1: int
      :param assignment2: Second assignment ID.
      :type assignment2: int
      :param cost: Pairwise cost of this edge.
      :type cost: float

      

   
   .. py:method:: add_edge(node1_left: int, node2_left: int, node1_right: int, node2_right: int, cost: float) -> None
      :noindex:

      
      Add an edge via four node IDs.

      :param node1_left: First node of first assignment.
      :type node1_left: int
      :param node2_left: Second node of first assignment.
      :type node2_left: int
      :param node1_right: First node of second assignment.
      :type node1_right: int
      :param node2_right: Second node of second assignment.
      :type node2_right: int
      :param cost: Pairwise cost of this edge.
      :type cost: float

      

   
   
   