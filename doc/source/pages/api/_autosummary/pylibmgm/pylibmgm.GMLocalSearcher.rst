pylibmgm.GMLocalSearcher
========================

.. currentmodule:: pylibmgm



.. autoclass:: pylibmgm.GMLocalSearcher
   :members:
   :member-order: bysource
   :exclude-members: __delattr__, __dir__, __eq__, __format__, __ge__, __getattribute__, __getstate__, __gt__, __hash__, __init_subclass__, __le__, __lt__, __ne__, __new__, __reduce__, __reduce_ex__, __repr__, __setattr__, __sizeof__, __str__, __subclasshook__, __weakref__, __dict__, __annotations__, __doc__, __module__, __init__

   
   
   
   .. rubric:: Methods

   .. autosummary::
   
      ~GMLocalSearcher.__init__
      ~GMLocalSearcher.search
   
   

   
   
   
   

   
   
   
   

   
   
   
   

   
   .. py:method:: __init__(model: MgmModel) -> None
      :noindex:

      
      Initialize with an MGM model.

      :param model: The MGM model.
      :type model: MgmModel

      

   
   .. py:method:: __init__(model: MgmModel, matching_order: list[int]) -> None
      :noindex:

      
      Initialize with an MGM model and search order.

      :param model: The MGM model.
      :type model: MgmModel
      :param matching_order: Order in which to re-match graphs.
      :type matching_order: list[int]

      

   
   
   