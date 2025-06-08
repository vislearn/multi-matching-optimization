# Multi-matching optimization
Application and library to solve (multi-)graph matching problems.

For licensing term, see the `LICENSE` file. This work uses third party software. Please refer to `LICENSE-3RD-PARTY.txt` for an overview and recognition.

For details, refer to our publication:

-   M. Kahl, S. Stricker, L. Hutschenreiter, F. Bernard, B. Savchynskyy<br>
    **“Unlocking the Potential of Operations Research for Multi-Graph Matching”**.<br>
    arXiv Pre-Print 2024. [[PDF][arxiv_pdf]] [[ArXiv][arxiv]]

### Python

**Install via pip**

    `pip install pylibmgm`

Refer to the usage guide: [Python Documentation][readthedocs]

### C++

0. (Install requirements)
    - Install `g++` or `clang` *(c++ compiler)*, `meson` *(build system)* and `git`
        - `sudo apt install g++ meson git` (Ubuntu)
        - `sudo dnf install g++ meson git` (Fedora)

1. **Clone repository**

    - `git clone https://github.com/vislearn/multi-matching-optimization.git`
    - `cd ./multi-matching-optimization`

3. **Build**

    - `meson setup --buildtype release -Db_ndebug=true ../builddir/`
    - `meson compile -C ../builddir/`

4. **Run**

    - `../builddir/mgm -i tests/hotel_instance_1_nNodes_10_nGraphs_4.txt -o ../mgm_output --mode seqseq`

# Usage

### Minimal Usage

    $ mgm -i [IN_FILE] -o [OUT_DIR] --mode [OPT_MODE]

You can use one of the small models in the `tests/` directory for testing.

### Input files
Input files follow the .dd file format for multi-graph matching problems, as defined in the [Structured prediction problem archive][problem_archive].
See references below.

### Available CLI parameters

-   `--name` <br>
    Base name for the resulting .json and .log files.

-   `-l`, `--labeling` <br>
    Path to an existing solution file (json). Pass to improve upon this labeling.

-   `--mode` ENUM <br>
    Optimization mode. See below.

-   `--set-size`INT <br>
    Subset size for incremenetal generation

-   `--merge-one` <br>
    In parallel local search, merge only the best solution. Do not try to merge other solutions as well.

-   `-t`, `--threads` INT <br>
    Number of threads to use. Upper limit defined by OMP_NUM_THREADS environment variable.

-   `--libmpopt-seed` UINT <br>
    Fix the random seed for the fusion moves graph matching solver of libmpopt.

-   `--unary-constant` FLOAT <br>
    Constant to add to every assignment cost. Negative values nudges matchings to be more complete.

-   `--synchronize` <br>
    Synchronize a cylce inconsistent solution.
    Excludes: --synchronize-infeasible

-   `--synchronize-infeasible` <br>
    Synchronize a cylce inconsistent solution. Allow all (forbidden) matchings.
    Excludes: --synchronize

### Optimization modes

`--mode` specifies the optimization routine. It provides ready to use routines that combine construction, graph matching local search (GM-LS), and swap local search (SWAP-LS) algorithms as defined in the publication.

The following choices are currently available:

***Construction modes.***
Use these to generate solutions quickly
- `seq`:                   sequential  construction
- `par`:                   parallel    construction
- `inc`:                   incremental construction

***Construction + GM-LS modes.***
A bit slower, but gives better solutions

- `seqseq`:                sequential  construction -> sequential GM-LS
- `seqpar`:                sequential  construction -> parallel   GM-LS
- `parseq`:                parallel    construction -> sequential GM-LS
- `parpar`:                parallel    construction -> parallel   GM-LS
- `incseq`:                incremental construction -> sequential GM-LS
- `incpar`:                incremental construction -> parallel   GM-LS

***Construction + iterative GM-LS & SWAP-LS.***
After construction, iterate between GM-LS and and SWAP-LS.

- `optimal`:               sequential  construction -> Until conversion: (sequential GM-LS <-> swap local search)
- `optimalpar`:            parallel    construction -> Until conversion: (parallel   GM-LS <-> swap local search)

***Improve given labeling.***

Skip construction and perform local search on a pre-existing solution.
Improve modes require a labeling to be provided via the `--labeling [JSON_FILE]` command line option.

- `improve-swap`:          improve with SWAP-LS
- `improve-qap`:           improve with sequential GM-LS
- `improve-qap-par`:       improve with parallel GM-LS
- `improveopt`:            improve with alternating sequential GM-LS <-> SWAP-LS
- `improveopt-par`:        improve with alternating parallel GM-LS <-> SWAP-LS

### Use as synchronization algorithm
To synchronize a pre-existing *cylce inconsistent* solution, call with `--synchonize` or `--synchonize-infeasible`, either disallowing or allowing forbidden matchings.

***Synchronize feasible.***
Feasible solution. Disallows forbidden matchings.

    $ mgm -i [IN_FILE] -o [OUT_DIR] --mode [OPT_MODE] --labeling [JSON_FILE] --synchonize

***Synchronize infeasible.***
Infeasible solution. Allows forbidden matchings.

    $ mgm -i [IN_FILE] -o [OUT_DIR] --mode [OPT_MODE] --labeling [JSON_FILE] --synchonize-infeasible

# Software dependencies
This software incorporates and uses third party software. Dependencies are provided as meson wrap files.
They should be installed automatically during the build process,
but manual installation can help solve issues, if meson fails to build them.

***NOTE: Links given point to the original authors publications, which may have been substantially modified. The corresponding `.wrap` files in this repositry's `subprojects/` folder contain links to the actual code needed to compile the solver.***

- **spdlog** \
    *logging library; https://github.com/gabime/spdlog*
- **cli11** \
    *Command line parser; https://github.com/CLIUtils/CLI11*
- **nlohman_json** \
    *json parsing. https://github.com/nlohmann/json*
- **unordered_dense** \
    *A fast & densely stored hashmap; https://github.com/martinus/unordered_dense*
- **libqpbo** \
    *for quadratic pseudo boolean optimization (QPBO); https://pub.ista.ac.at/~vnk/software.html*
- **libmpopt** \
    *for quadratic assignment problem (QAP) optimization; https://github.com/vislearn/libmpopt*
- **Scipy_lap** \
    *Linear assignment problem (LAP) solver implementation from the Scipy python package; https://github.com/scipy/scipy*

# References

-   M. Kahl, S. Stricker, L. Hutschenreiter, F. Bernard, B. Savchynskyy<br>
    **“Unlocking the Potential of Operations Research for Multi-Graph Matching”**.<br>
    arXiv Pre-Print 2024. [[PDF][arxiv_pdf]] [[ArXiv][arxiv]]

-   L. Hutschenreiter, S. Haller, L. Feineis, C. Rother, D. Kainmüller, B. Savchynskyy.<br>
    **“Fusion Moves for Graph Matching”**.<br>
    ICCV 2021. [[PDF][iccv2021_pdf]] [[ArXiv][iccv2021]]

-   Swoboda, P., Andres, B., Hornakova, A., Bernard, F., Irmai, J., Roetzer, P., Savchynskyy, B., Stein, D. and Abbas, A. <br>
    **“Structured prediction problem archive”** <br>
    arXiv preprint arXiv:2202.03574 (2022) [[PDF][problem_archive]] [[ArXiv][problem_archive_pdf]]

[arxiv]:                https://arxiv.org/abs/2406.18215
[arxiv_pdf]:            https://arxiv.org/pdf/2406.18215
[iccv2021]:             https://arxiv.org/abs/2101.12085
[iccv2021_pdf]:         https://arxiv.org/pdf/2101.12085
[problem_archive]:      https://arxiv.org/abs/2202.03574
[problem_archive_pdf]:  https://arxiv.org/pdf/2202.03574
[readthedocs]:          https://pylibmgm.readthedocs.io/en/latest/