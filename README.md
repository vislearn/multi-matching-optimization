# Multi-graph matching C++ implementation

# Requirements
The following requirements are not installed automatically during the later installation.
Build process will throw errors, if not present.

Requirements:
- Meson
- Ninja
- C++ 17
- spdlog (logging library; https://github.com/gabime/spdlog; Also available as meson wrap)
- fmtlib (Pre C++20 <format> library implementation; https://github.com/fmtlib/fmt; Also available as meson wrap)
- cli11 (Command line parser; https://github.com/CLIUtils/CLI11; Also available as meson wrap)

# Installation
This project has dependencies on the following libraries:
-   ``libqpbo``,            for quadratic pseudo boolean optimization (QPBO).
-   ``libmpopt``,           for quadratic assignment problem (QAP) optimization.
-   ``nlohman_json``,       for parsing json objects.

Initialize them into subprojects/ folder via
-   ``git submodule init``
-   ``git submodule update``

Then build and compile with meson
-   ``meson setup builddir``
-   ``meson compile -C builddir``

The build directory then contains an exectuable to use the software.

# Usage
...