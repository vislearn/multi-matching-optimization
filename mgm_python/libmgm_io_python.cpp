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

PYBIND11_MODULE(io, m_io)
{   
    m_io.def("parse_dd_file", &mgm::io::parse_dd_file,
            py::arg("dd_file"),
            py::arg("unary_constant") = 0.0);
            
    m_io.def("parse_dd_file_gm", &mgm::io::parse_dd_file_gm,
            py::arg("dd_file"),
            py::arg("unary_constant") = 0.0);

    m_io.def("save_to_disk", py::overload_cast<std::filesystem::path, const mgm::MgmSolution&>(&mgm::io::save_to_disk), py::doc(save_to_disk_gm_doc));
    m_io.def("save_to_disk", py::overload_cast<std::filesystem::path, const mgm::GmSolution&>(&mgm::io::save_to_disk) , py::doc(save_to_disk_doc));
    m_io.def("export_dd_file", &mgm::io::export_dd_file);
    m_io.def("import_solution", &mgm::io::import_from_disk);

    m_io.def("_register_io_logger", &register_python_logger, "Register a Python logger with spdlog");
}