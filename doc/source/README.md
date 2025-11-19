# Documentation

## API Documentation

The API documentation section was generated iteratively using GitHub Copilot and Claude Sonnet 4.5.
In specific:
    _templates/autosummary/class.rst
    _templates/autosummary/function.rst
    _ext/stub_autodoc
    conf.py (partially)
    pages/api/... (partially)

### Custom Sphinx Extension: `stub_autodoc`

This project uses a custom Sphinx extension (`stub_autodoc.py`) to extract documentation from `.pyi` stub files instead of runtime docstrings. This was necessary because infering all necessary class/function/type information and documentation from c++ binding code automatically prooved difficult.
Furthermore, `.pyi` stub files integrate well with IDE inspection tools.

Therefore:

- The Python bindings are generated using pybind11 from C++ code. This defines interface between python and c++.
- Stub files (`.pyi`) contain complete type annotations and detailed docstrings for these interfaces.

However, for pure Python modules (e.g. solver.py) the documentation is written in-file as is expected.

The extension handles overloaded functions/methods, nested classes, properties, and Napoleon-style docstrings, generating clean and compact API documentation.

In the end, this is not very ideal and I expect the custom parser to cause issues in the future, as it seems very hardcoded for the current documentation. 
If someone finds a way of integrating the documentation more nicely, directly into the pybind11 interface and have parsing done automatically, that would make it a lot simpler.