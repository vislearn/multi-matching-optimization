#include <libmgm/mgm.hpp>

#include <filesystem>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl/filesystem.h>

#include "logging_adapter.hpp"

namespace py = pybind11;
namespace fs = std::filesystem;


// --- Docstrings section ---
constexpr const char* parse_dd_file_doc = R"doc(
    Load a .dd file from disk containing an MGM model definition.

    Several sentences providing an extended description. Refer to
    variables using back-ticks, e.g. `var`.

    Parameters
    ----------
    dd_file : os.PathLike
        The path to the .dd file to be loaded. This can be a string or
        a `pathlib.Path` object. The file should be in the
        Multi-Graph Matching model format specified in [1]_.
    unary_constant : float, optional
        A constant value to be added to the unary costs in the model.

    Returns
    -------
    :class:`pylibmgm.MgmModel`    

    References
    ----------
    .. [1] P. Swoboda et al., "Structured Prediction Problem Archive",
            ArXiv, 2023, https://arxiv.org/abs/2202.03574

)doc";

constexpr const char* parse_dd_file_gm_doc = R"doc(
    Load a .dd file from disk containing a GM model definition.

    Parameters
    ----------
    dd_file : os.PathLike
        The path to the .dd file to be loaded. This can be a string or
        a `pathlib.Path` object. The file should be in the
        Graph Matching model format specified in [1]_.
    unary_constant : float, optional
        A constant value to be added to the unary costs in the model.

    Returns
    -------
    :class:`.pylibmgm.GmModel`

    References
    ----------
    .. [1] P. Swoboda et al., "Structured Prediction Problem Archive",
            ArXiv, 2023, https://arxiv.org/abs/2202.03574

)doc";


constexpr const char* save_to_disk_gm_doc = R"doc(
    Store a GM solution in json format on disk.

    Parameters
    ----------
    filepath : os.PathLike
        If filepath is a directory, the solution will be stored in a generically named file.
        Optionally, include the filename in the ``filepath`` argument to control the output file name.
    solution : :class:`pylibmgm.GmSolution`

)doc";

constexpr const char* save_to_disk_doc = R"doc(
    Store a MGM solution in json format on disk.

    Parameters
    ----------
    filepath : os.PathLike
        If filepath is a directory, the solution will be stored in a generically named file.
        Optionally, include the filename in the ``filepath`` argument to control the output file name.
    solution : :class:`pylibmgm.MgmSolution`

)doc";

constexpr const char* export_dd_file_doc = R"doc(
    Exports a given MGM model to a .dd file.

    See [1]_ for details on the .dd file format.

    Parameters
    ----------
    filepath : os.PathLike
        Must not be a directory, but a file path.
    model : :class:`pylibmgm.MgmModel`

    References
    ----------
    .. [1] P. Swoboda et al., "Structured Prediction Problem Archive",
            ArXiv, 2023, https://arxiv.org/abs/2202.03574

)doc";

constexpr const char* import_solution_doc = R"doc(
    Load a solution for a given MgmModel from disk.

    To create a solution file for loading with this function, use 
    the :func:`pylibmgm.io.save_to_disk` function.

    Parameters
    ----------
    solution_path : os.PathLike
        Path to the solution file. 
    model : :class:`pylibmgm.MgmModel`

    Returns
    -------
    :class:`pylibmgm.MgmSolution`

)doc";

PYBIND11_MODULE(io, m_io)
{   
    m_io.def("parse_dd_file", &mgm::io::parse_dd_file,
            py::arg("dd_file"),
            py::arg("unary_constant") = 0.0, 
        py::doc(parse_dd_file_doc));
            
    m_io.def("parse_dd_file_gm", &mgm::io::parse_dd_file_gm,
            py::arg("dd_file"),
            py::arg("unary_constant") = 0.0,
            py::doc(parse_dd_file_gm_doc));
            

    m_io.def("save_to_disk", py::overload_cast<std::filesystem::path, const mgm::MgmSolution&>(&mgm::io::save_to_disk), py::doc(save_to_disk_gm_doc));
    m_io.def("save_to_disk", py::overload_cast<std::filesystem::path, const mgm::GmSolution&>(&mgm::io::save_to_disk) , py::doc(save_to_disk_doc));
    m_io.def("export_dd_file", &mgm::io::export_dd_file, py::doc(export_dd_file_doc)); // TODO: Write function for GM Model as well.
    m_io.def("import_solution", &mgm::io::import_from_disk, py::doc(import_solution_doc));

    m_io.def("_register_io_logger", &register_python_logger, "Register a Python logger with spdlog");
}