pylibmgm.MgmSolution
====================

.. currentmodule:: pylibmgm



.. autoclass:: pylibmgm.MgmSolution
   :members:
   :member-order: bysource
   :exclude-members: __delattr__, __dir__, __eq__, __format__, __ge__, __getattribute__, __getstate__, __gt__, __hash__, __init_subclass__, __le__, __lt__, __ne__, __new__, __reduce__, __reduce_ex__, __repr__, __setattr__, __sizeof__, __str__, __subclasshook__, __weakref__, __dict__, __annotations__, __doc__, __module__, model, evaluate, set_solution

   
   
   
   .. rubric:: Methods

   .. autosummary::
   
      ~MgmSolution.__init__
      ~MgmSolution.cliques
      ~MgmSolution.create_empty_labeling
      ~MgmSolution.evaluate
      ~MgmSolution.labeling
      ~MgmSolution.set_solution
      ~MgmSolution.to_dict_with_none
   
   

   
   
   
   .. rubric:: Attributes

   .. list-table::
      :widths: 25 20 55
      :header-rows: 1

      * - Name
        - Type
        - Description
      
      * - **model**
        - :class:`~pylibmgm.MgmModel`

        - The MGM model for this solution.
   
   

   
   
   
   
   
   

   
   
   
   

   
   .. py:method:: evaluate() -> float
      :noindex:

      
      Evaluate total objective value across all pairwise models.

      :returns: Total objective value.
      :rtype: float

      

   
   .. py:method:: evaluate(graph_id: int) -> float
      :noindex:

      
      Evaluate objective restricted to models involving a specific graph.

      :param graph_id: Graph ID to evaluate.
      :type graph_id: int

      :returns: Objective value for models involving this graph.
      :rtype: float

      

   

   
   .. py:method:: set_solution(labeling: dict[tuple[int, int], list[int]]) -> None
      :noindex:

      
      Set solution from a labeling dictionary.
      

   
   .. py:method:: set_solution(graph_pair: tuple[int, int], labeling: list[int]) -> None
      :noindex:

      
      Set solution for a specific pairwise model.
      

   
   .. py:method:: set_solution(gm_solution: GmSolution) -> None
      :noindex:

      
      Update solution with a pairwise GM solution.
      

   
   
   