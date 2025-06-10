pylibmgm documentation
======================
| This is the documentation for `pylibmgm`, a Python wrapper for the `libmgm` C++ library.
| See our `github repository <https://github.com/vislearn/multi-matching-optimization>`_ for reference.

If you use this library in your work, please cite our paper [1]_.

.. toctree::
    :hidden:

    Home <self>

    
.. toctree::
    :maxdepth: 1

    pages/quickstart
    pages/API

Why pylibmgm?
-------------
Pylibmgm contains state-of-the-art solvers for Graph Matching and Multi-Graph Matching problems,
wrapped in an easy-to-use Python interface.

For Graph Matching, we provide the dual Fusion Moves solver [2]_. For Multi-Graph Matching, we provide GREEDA [1]_, 
our newest solver which prompted us to create this package.

In contrast to other competitor algorithms, our solvers:

- can handle *sparse costs*
- directly apply to *incomplete problems*
- guarantee *cycle-consistency* (Multi-Graph Matching)

This allows them to scale to problems with hundreds of nodes and objects.

For details, please refer to the respective publications [1]_ and [2]_.

.. [1] M\. Kahl, S\. Stricker, L\. Hutschenreiter, F\. Bernard, C\. Rother, B\. Savchynskyy. *Towards Optimizing Large-Scale Multi-Graph Matching in Bioimaging*. Proceedings of the Computer Vision and Pattern Recognition Conference (CVPR), 2025.

.. [2] L\. Hutschenreiter, S\. Haller, L\. Feineis, C\. Rother, D\. Kainmüller, B\. Savchynskyy. *Fusion Moves for Graph Matching*. Proceedings of the IEEE/CVF International Conference on Computer Vision (ICCV), 2021.
