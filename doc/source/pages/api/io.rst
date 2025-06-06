pylibmgm.io
===========
.. currentmodule:: pylibmgm.io

.. autosummary::
    :toctree: _autosummary/io
    :signatures: short

    parse_dd_file
    parse_dd_file_gm
    export_dd_file
    import_solution

Additional, overloaded functions:

.. toctree::
    :maxdepth: 1

    overloaded_functions/save_to_disk


.. This function is overloaded:

.. - One version accepts an `MgmSolution`.
.. - Another accepts a `GmSolution`.

.. .. py:function:: safe_to_disk(path: pathlib.Path, solution: MgmSolution)

..    Save MgmSolution to disk.

.. .. py:function:: safe_to_disk(path: pathlib.Path, solution: GmSolution)
..    :noindex:

..    Save GmSolution to disk.