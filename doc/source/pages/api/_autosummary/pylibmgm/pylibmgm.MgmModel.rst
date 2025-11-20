pylibmgm.MgmModel
=================

.. currentmodule:: pylibmgm



.. autoclass:: pylibmgm.MgmModel
   :members:
   :member-order: bysource
   :exclude-members: __delattr__, __dir__, __eq__, __format__, __ge__, __getattribute__, __getstate__, __gt__, __hash__, __init_subclass__, __le__, __lt__, __ne__, __new__, __reduce__, __reduce_ex__, __repr__, __setattr__, __sizeof__, __str__, __subclasshook__, __weakref__, __dict__, __annotations__, __doc__, __module__, graphs, models, no_graphs

   
   
   
   .. rubric:: Methods

   .. autosummary::
   
      ~MgmModel.__init__
      ~MgmModel.add_model
      ~MgmModel.create_submodel
   
   

   
   
   
   .. rubric:: Attributes

   .. list-table::
      :widths: 25 20 55
      :header-rows: 1

      * - Name
        - Type
        - Description
      
      * - **graphs**
        - list[:class:`~pylibmgm.Graph`]

        - List of all graphs.  IDs graph.id should be sorted in ascending order starting from zero: (0,1,2,3,4,...).
      * - **models**
        - dict[tuple[int, int], :class:`~pylibmgm.GmModel`]

        - Dictionary of pairwise GM models indexed by (graph1_id, graph2_id).
      * - **no_graphs**
        - int

        - Number of graphs in the problem.
   
   

   
   
   
   
   
   

   
   
   
   
   
   