# Multi-graph matching C++ implementation

# Requirements
Requirements:
- Meson
- Ninja
- C++ 17
- spdlog (logging library; https://github.com/gabime/spdlog)
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