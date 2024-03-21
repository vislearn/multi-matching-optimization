# Multi-graph matching C++ implementation
Application and library to solve (multi-)graph matching problems.

For licensing term, see the `LICENSE` file. This work uses third party software. Please refer to `LICENSE-3RD-PARTY.txt` for an overview and recognition.

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
    *A fast & densely stored hashmap*

Initialize them into the subprojects/ folder via
-   ``git submodule init``
-   ``git submodule update``

### Build-time Requirements
All further dependencies are provided as meson wrap files:

- **spdlog** \
    *logging library; https://github.com/gabime/spdlog*
- **fmtlib** \
    *Pre C++20 \<format\> library implementation; https://github.com/fmtlib/fmt*
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
...