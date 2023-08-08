# Multi-graph matching C++ implementation

# Requirements
Requirements:
- Meson
- Ninja
- C++ 17
- spdlog (logging library; https://github.com/gabime/spdlog)
# Installation

Initialize the ``libqpbo`` and ``libmpopt`` dependencies into subprojects/ folder.
-   ``git submodule init``
-   ``git submodule update``

Build with meson
-   ``meson setup builddir``
-   ``meson compile -C builddir``