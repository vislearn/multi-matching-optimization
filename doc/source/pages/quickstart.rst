Quickstart Guide
======================

Installation
----------------------

You can install `pylibmgm` directly via pip from PyPI:

.. code-block:: bash

    pip install pylibmgm

If you encounter any issues with the pre-built wheels, please do open an issue
on our `github repository <https://github.com/vislearn/multi-matching-optimization>`_.
We highly appreciate any help to keep this library working on all platforms.

You can test that the package is working by grabbing 
a `test model <https://github.com/vislearn/multi-matching-optimization/tree/main/tests>`_ 
from our repository and running:

.. code-block:: python

    import pylibmgm

    model = pylibmgm.io.parse_dd_file("tests/hotel_instance_1_nNodes_10_nGraphs_4.txt")
    solution = pylibmgm.solver.solve_mgm(model)

    print("Solution cost:", solution.evaluate())
    print("Labeling:", solution.labeling())

Usage
----------------------

Using pylibmgm typically involves three steps:

    1. Creating a problem model.
    2. Running a solver.
    3. Retrieving the solution.

Step 1: Creating a problem model
+++++++++++++++++++++++++++++++++++++

To create a problem model, you have two options:

    - Loading a model description from a file.
    - Creating a model on the fly.

**Loading a model description from a file**

Pylibmgm can import models in the file format described in [1]_. For examples, take a look at the 
`test models <https://github.com/vislearn/multi-matching-optimization/tree/main/tests>`_ in our github repository.

Loading a model is as simple as:

.. code-block:: python

    import pylibmgm

    # Load an MGM model from a file
    model = pylibmgm.io.parse_dd_file("path/to/model.dd")

    # Load an GM model from a file
    gm_model = pylibmgm.io.parse_dd_file_gm("path/to/model.dd")

**Creating a model on the fly**

You can also add costs to a model iteratively. However, it is necessary to
define a limit for the amount of linear and quadratic costs beforehand.

.. code-block:: python

    import pylibmgm

    # Define graphs
    graph_0 = pylibmgm.Graph(0, 2)  # Graph with ID 0 and 2 nodes
    graph_1 = pylibmgm.Graph(1, 3)  # Graph with ID 1 and 3 nodes

    # Create a Graph-Matching model for graph 0 and graph 1.
    # Reserve buffer for up to 10 linear and 100 quadratic costs.
    gm_model_1 = pylibmgm.GmModel(graph_0, graph_1, 10, 100)

    # Add linear costs
    # add_assignment(node1, node2, cost)
    gm_model_1.add_assignment(0, 0, -1.0)
    gm_model_1.add_assignment(0, 1, 2.0)
    gm_model_1.add_assignment(0, 2, 3.0)
    gm_model_1.add_assignment(1, 0, 1.0)
    gm_model_1.add_assignment(1, 1, -1.0)
    gm_model_1.add_assignment(1, 2, 3.0)

    # Add quadratic costs
    # add_edge(node_ass1_graph1, node_ass1_graph2, node_ass2_graph1, node_ass2_graph2, cost)
    gm_model_1.add_edge(0, 0, 1, 1, 0.5)
    gm_model_1.add_edge(0, 1, 1, 2, 2)
    ...

    # Compose the MgmModel from multiple GmModels
    mgm_model = pylibmgm.MgmModel()
    mgm_model.add_model(gm_model_1)
    mgm_model.add_model(gm_model_2)
    mgm_model.add_model(gm_model_3)
    ...


Step 2: Running a solver
+++++++++++++++++++++++++++++++++++++

Predefined solver routines are available in the :doc:`api/solver` module.

To **Solve a Graph Matching problem**, you can use the `solve_gm` function:

.. code-block:: python

    import pylibmgm

    gm_model = ... # create or load gm_model

    # Solve the model
    solution = pylibmgm.solver.solve_gm(gm_model)


To **Solve a Multi-Graph Matching problem**, you can use the `solve_mgm` function:

.. code-block:: python

    import pylibmgm

    mgm_model = ... # create or load mgm_model

    # Solve the model
    solution = pylibmgm.solver.solve_mgm(mgm_model)

    # Solve the model using parallel processing
    solution = pylibmgm.solver.solve_mgm_parallel(mgm_model, nr_threads=4)

Depending on your runtime budget, you can choose between several different levels of optimization.
See :doc:`api/_autosummary/solver/pylibmgm.solver.OptimizationLevel` for details.

If you can, we recommend you to always prefer `solve_mgm_parallel()`, 
as its solutions are usually of higher quality.

Step 3: Retrieve a solution
+++++++++++++++++++++++++++++++++++++

After running a solver, you may inspect or post-process the solution as you wish,
or alternatively, save it to a file on disk.

.. code-block:: python

    import pylibmgm

    model = ... # create or load gm_model

    solution = ... # run a solver

    # Inspect solution
    print("Solution cost:", solution.evaluate())
    print("Labeling:", solution.labeling())

    # Save solution to a file
    pylibmgm.io.save_to_disk(solution, "path/to/solution.dd")

Logging
----------------------
Pylibmgm uses spdlog for the C++ logging backend. 
For convenience, it is linked to the default Python logging module and you can integrate it into your own logging setup.

There are two python loggers available with which you can control the amount of logging that is output.

Import the python logging module via `import logging` and toggle the loggers:

**pylibmgm.io & all C++ backend logs:** 

.. code-block:: python

    import logging

    # Disable
    logging.getLogger("libmgm").setLevel(logging.ERROR)
    # Enable
    logging.getLogger("libmgm").setLevel(logging.INFO)

**pylibmgm.solver:**

.. code-block:: python

    import logging
    
    # Disable
    logging.getLogger("libmgm.interface").setLevel(logging.ERROR)
    # Enable
    logging.getLogger("libmgm.interface").setLevel(logging.INFO)


.. [1] P. Swoboda et al\., "Structured Prediction Problem Archive",
            ArXiv, 2023, https://arxiv.org/abs/2202.03574
