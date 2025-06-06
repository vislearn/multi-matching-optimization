pylibmgm.io.save_to_disk
========================

Overloaded function: Can be used with both :class:`pylibmgm.MgmSolution` and :class:`pylibmgm.GmSolution` types.

.. currentmodule:: pylibmgm.io
    
.. py:function:: save_to_disk(path: pathlib.Path, solution: MgmSolution)

    Store an MGM solution in JSON format on disk.

    :param filepath: If `path` is a directory, the solution will be stored in a generically named file.
                 To specify the output filename, include it in the `path` argument.
    :type filepath: os.PathLike

    :param solution:
    :type solution: :class:`pylibmgm.MgmSolution`

.. py:function:: save_to_disk(path: pathlib.Path, solution: GmSolution)
   :noindex:

   Same as above, but for :class:`pylibmgm.GmSolution`.