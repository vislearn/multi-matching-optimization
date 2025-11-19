pylibmgm.CostMap
================

.. currentmodule:: pylibmgm



.. autoclass:: pylibmgm.CostMap
   :members:
   :member-order: bysource
   :exclude-members: __delattr__, __dir__, __eq__, __format__, __ge__, __getattribute__, __getstate__, __gt__, __hash__, __init_subclass__, __le__, __lt__, __ne__, __new__, __reduce__, __reduce_ex__, __repr__, __setattr__, __sizeof__, __str__, __subclasshook__, __weakref__, __dict__, __annotations__, __doc__, __module__, contains, pairwise, unary

   
   
   
   .. rubric:: Methods

   .. autosummary::
   
      ~CostMap.__init__
      ~CostMap.contains
      ~CostMap.pairwise
      ~CostMap.unary
   
   

   
   
   
   

   
   
   
   

   
   
   
   

   
   .. py:method:: contains(node1: int, node2: int) -> bool
      :noindex:

      
      Check if unary cost exists for given node pair.
      

   
   .. py:method:: contains(assignment: tuple[int, int]) -> bool
      :noindex:

      
      Check if unary cost exists for given assignment.
      

   
   .. py:method:: contains(node1_left: int, node2_left: int, node1_right: int, node2_right: int) -> bool
      :noindex:

      
      Check if pairwise cost exists for given node quadruple.
      

   
   .. py:method:: contains(edge: tuple[tuple[int, int], tuple[int, int]]) -> bool
      :noindex:

      
      Check if pairwise cost exists for given edge.
      

   

   
   .. py:method:: pairwise(node1_left: int, node2_left: int, node1_right: int, node2_right: int) -> float
      :noindex:

      
      Get pairwise cost for edge defined by four node indices.
      

   
   .. py:method:: pairwise(edge: tuple[tuple[int, int], tuple[int, int]]) -> float
      :noindex:

      
      Get pairwise cost for an edge index.
      

   

   
   .. py:method:: unary(node1: int, node2: int) -> float
      :noindex:

      
      Get unary cost for matching node1 (from graph1) to node2 (from graph2).
      

   
   .. py:method:: unary(assignment: tuple[int, int]) -> float
      :noindex:

      
      Get unary cost for an assignment index.
      

   
   
   