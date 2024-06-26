# Multi-graph matching C++ implementation
Application and library to solve (multi-)graph matching problems.

For licensing term, see the `LICENSE` file. This work uses third party software. Please refer to `LICENSE-3RD-PARTY.txt` for an overview and recognition.

## Quickstart (tl;dr)

0. (Install requirements)
    - Install `g++` or `clang` *(c++ compiler)*, `git` *(for submodules)* and `meson` *(build system)*
        - `sudo apt install g++ git meson ` (Ubuntu)
        - `sudo dnf install g++ git meson` (Fedora)

1. Clone repository 

    - `git clone https://github.com/vislearn/multi-matching-optimization.git`
    - `cd ./multi-matching-optimization`

2. Initialize submodules (dependencies)

    - ``git submodule init``
    - ``git submodule update``

3. Build

    - ``meson setup builddir/``
    - ``meson compile -C builddir/``

4. Run
    
    - `cd builddir/`
    - `mgm -i [IN_FILE] -o [OUT_DIR] --mode [OPT_MODE]`

# Installation
To run this software, follow the guide below to install all necessary requirements and build the application.

## Requirements

### Prequisite Requirements
The following requirements are not installed automatically during installation.
Build process will throw errors, if not present.

- **C++ 17 compiler**
- **Meson** \
    *https://mesonbuild.com/*
- **Ninja** \
    *Typically ships with meson. https://ninja-build.org/*

### Git submodule Requirements
The following libraries are managed as git submodules:

-   **libqpbo** \
    *for quadratic pseudo boolean optimization (QPBO).*
-   **libmpopt** \
    *for quadratic assignment problem (QAP) optimization.*
-   **unordered_dense** \
    *A fast & densely stored hashmap.*
-   **Scipy_lap** \
    *Linear assignment problem (LAP) solver implementation from the Scipy python package.*

Initialize them into the subprojects/ folder via
-   ``git submodule init``
-   ``git submodule update``

### Build-time Requirements
All further dependencies are provided as meson wrap files:

- **spdlog** \
    *logging library; https://github.com/gabime/spdlog*
- **cli11** \
    *Command line parser; https://github.com/CLIUtils/CLI11*
- **nlohman_json** \
    *json parsing. https://github.com/nlohmann/json*

They should be installed automatically during the build process,
but manual installation can help solve issues, if meson fails to build them.
## Building

Build and compile with meson
1.   ``meson setup builddir/``
2.   ``meson compile -C builddir/``

The build directory then contains the `mgm` exectuable.

# Usage

### Minimal Usage

    $ mgm -i [IN_FILE] -o [OUT_DIR] --mode [OPT_MODE]

Call `mgm --help` for an overview of all command line options.
You can use one of the small models in the `test_models/` directory for testing.

### Input files
Input files follow the .dd file format for multi-graph matching problems, as defined in the [Structured prediction problem archive][problem_archive]:

    Swoboda, P., Andres, B., Hornakova, A., Bernard, F., Irmai, J., Roetzer, P., Savchynskyy, B., Stein, D. and Abbas, A.
    “Structured prediction problem archive”
    arXiv preprint arXiv:2202.03574 (2022)

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



[problem_archive]: https://arxiv.org/abs/2202.03574