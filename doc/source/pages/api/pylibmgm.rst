pylibmgm
========

The ``pylibmgm`` module provides low-level building blocks for multi-graph matching problems. 
This API is useful when you need fine-grained control over the optimization process or want to 
implement custom solvers beyond what is provided in :doc:`solver`.

.. currentmodule:: pylibmgm

Core Data Structures
--------------------

Graph and Model Classes
~~~~~~~~~~~~~~~~~~~~~~~~

These classes represent the fundamental data structures for graph matching problems.

.. autosummary::
    :toctree: _autosummary/pylibmgm

    Graph
    GmModel
    MgmModel
    CostMap

Solution Classes
~~~~~~~~~~~~~~~~

Classes for representing and manipulating solutions to matching problems.

.. autosummary::
    :toctree: _autosummary/pylibmgm

    GmSolution
    MgmSolution

Solvers and Optimization
-------------------------

Graph Matching - Solvers
~~~~~~~~~~~~~~~~~~~~~~~~

Solvers for pairwise graph matching (GM) problems.

.. autosummary::
    :toctree: _autosummary/pylibmgm

    LAPSolver
    QAPSolver

Multi Graph Matching - Solution Generators
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Generators for constructing initial multi-graph matching solutions.

.. autosummary::
    :toctree: _autosummary/pylibmgm

    MgmGenerator.matching_order
    SequentialGenerator
    ParallelGenerator


Multi Graph Matching- Local Search Methods
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Local search algorithms for refining solutions.

.. autosummary::
    :toctree: _autosummary/pylibmgm

    GMLocalSearcher
    GMLocalSearcherParallel
    SwapLocalSearcher

Utility Functions
-----------------

.. autosummary:: 
    :toctree: _autosummary/pylibmgm

    build_sync_problem
    omp_set_num_threads