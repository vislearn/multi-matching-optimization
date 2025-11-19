pylibmgm.io.save\_to\_disk
==========================

.. currentmodule:: pylibmgm.io





.. py:function:: save_to_disk(filepath: os.PathLike, solution: pylibmgm.MgmSolution) -> None



   Save a MGM solution to disk in JSON format.

   :param filepath: If filepath is a directory, the solution will be stored in a generically named file.
                    Optionally, include the filename in the filepath to control the output file name.
   :type filepath: os.PathLike
   :param solution: The MGM solution to save.
   :type solution: MgmSolution or GmSolution




.. py:function:: save_to_disk(filepath: os.PathLike, solution: pylibmgm.GmSolution) -> None
   :noindex:



   Save a GM solution to disk in JSON format.

   :param filepath: If filepath is a directory, the solution will be stored in a generically named file.
                    Optionally, include the filename in the filepath to control the output file name.
   :type filepath: os.PathLike
   :param solution: The GM solution to save.
   :type solution: MgmSolution or GmSolution





