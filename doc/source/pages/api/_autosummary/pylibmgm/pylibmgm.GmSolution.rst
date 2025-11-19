pylibmgm.GmSolution
===================

.. currentmodule:: pylibmgm



.. autoclass:: pylibmgm.GmSolution
   :members:
   :member-order: bysource
   :exclude-members: __delattr__, __dir__, __eq__, __format__, __ge__, __getattribute__, __getstate__, __gt__, __hash__, __init_subclass__, __le__, __lt__, __ne__, __new__, __reduce__, __reduce_ex__, __repr__, __setattr__, __sizeof__, __str__, __subclasshook__, __weakref__, __dict__, __annotations__, __doc__, __module__, model, __init__

   
   
   
   .. rubric:: Methods

   .. autosummary::
   
      ~GmSolution.__init__
      ~GmSolution.evaluate
      ~GmSolution.evaluate_static
      ~GmSolution.labeling
      ~GmSolution.to_list_with_none
   
   

   
   
   
   .. rubric:: Attributes

   .. list-table::
      :widths: 25 20 55
      :header-rows: 1

      * - Name
        - Type
        - Description
      
      * - **model**
        - :class:`~pylibmgm.GmModel`

        - The GM model for this solution.
   
   

   
   
   
   
   
   

   
   
   
   

   
   .. py:method:: __init__() -> None
      :noindex:

      
      Create an empty GM solution.
      

   
   .. py:method:: __init__(model: GmModel) -> None
      :noindex:

      
      Create a GM solution for a model.

      :param model: The GM model this solution belongs to.
      :type model: GmModel

      

   
   .. py:method:: __init__(model: GmModel, labeling: list[int]) -> None
      :noindex:

      
      Create a GM solution with initial labeling.

      :param model: The GM model this solution belongs to.
      :type model: GmModel
      :param labeling: Initial labeling. For each node i in graph1, labeling[i] is the matched
                       node in graph2, or -1 if unmatched.
      :type labeling: list[int]

      

   
   
   