pylibmgm.QAPSolver
==================

.. currentmodule:: pylibmgm



.. autoclass:: pylibmgm.QAPSolver
   :members:
   :member-order: bysource
   :exclude-members: __delattr__, __dir__, __eq__, __format__, __ge__, __getattribute__, __getstate__, __gt__, __hash__, __init_subclass__, __le__, __lt__, __ne__, __new__, __reduce__, __reduce_ex__, __repr__, __setattr__, __sizeof__, __str__, __subclasshook__, __weakref__, __dict__, __annotations__, __doc__, __module__, default_run_settings, default_stopping_criteria, libmpopt_seed, run_settings, stopping_criteria

   
   
   
   .. rubric:: Methods

   .. autosummary::
   
      ~QAPSolver.__init__
      ~QAPSolver.run
   
   

   
   
   
   .. rubric:: Attributes

   .. list-table::
      :widths: 25 20 55
      :header-rows: 1

      * - Name
        - Type
        - Description
      
      * - **default_run_settings**
        - :ref:`RunSettings <pylibmgm.QAPSolver.RunSettings>`

        - Default run settings.
      * - **default_stopping_criteria**
        - :ref:`StoppingCriteria <pylibmgm.QAPSolver.StoppingCriteria>`

        - Default stopping criteria.
      * - **libmpopt_seed**
        - int

        - Random seed for the solver.
      * - **run_settings**
        - :ref:`RunSettings <pylibmgm.QAPSolver.RunSettings>`

        - Run configuration settings.
      * - **stopping_criteria**
        - :ref:`StoppingCriteria <pylibmgm.QAPSolver.StoppingCriteria>`

        - Stopping criteria for the solver.
   
   

   
   
   
   

   .. _pylibmgm.QAPSolver.RunSettings:

   **RunSettings**

   Configuration for the QAP solver run parameters.

   .. list-table::
      :widths: 30 20 50
      :header-rows: 1

      * - Attribute
        - Type
        - Description
      * - ``batch_size``
        - ``int``
        - Number of solutions per batch (default: 10).
      * - ``greedy_generations``
        - ``int``
        - Number of greedy generation steps (default: 10).

   .. _pylibmgm.QAPSolver.StoppingCriteria:

   **StoppingCriteria**

   Stopping criteria for the QAP solver.

   .. list-table::
      :widths: 30 20 50
      :header-rows: 1

      * - Attribute
        - Type
        - Description
      * - ``p``
        - ``int``
        - Convergence threshold parameter (default: 0.6).
      * - ``k``
        - ``int``
        - Number of iterations to check convergence.
      * - ``max_batches``
        - ``int``
        - Maximum number of batches to run.
   
   

   
   
   
   
   
   