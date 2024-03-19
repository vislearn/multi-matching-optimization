# Multi-graph matching C++ implementation

# Requirements
The following requirements are not installed automatically during the later installation.
Build process will throw errors, if not present.

Requirements:
- C++ 17
- Meson
- Ninja

All further dependencies are provided as meson wrap files:

- spdlog        *(logging library; https://github.com/gabime/spdlog)*
- fmtlib        *(Pre C++20 \<format\> library implementation; https://github.com/fmtlib/fmt)*
- cli11         *(Command line parser; https://github.com/CLIUtils/CLI11)*
- nlohman_json  *(json parsing. https://github.com/nlohmann/json)*

They should be installed automatically during the build process,
but manual installation can help solve issues, if meson fails to build them.

# Installation
This project has dependencies on the following libraries:
-   ``libqpbo``,            for quadratic pseudo boolean optimization (QPBO).
-   ``libmpopt``,           for quadratic assignment problem (QAP) optimization.

They are configured as git submodules. Initialize them into subprojects/ folder via
-   ``git submodule init``
-   ``git submodule update``

Then build and compile with meson
-   ``meson setup builddir``
-   ``meson compile -C builddir``

The build directory then contains an exectuable to use the software.

# Usage
...