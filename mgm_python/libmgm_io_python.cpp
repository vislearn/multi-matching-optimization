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


// --- C++ Binding Documentation ---
// NOTE: Full API documentation is maintained in mgm_python/stubs/pylibmgm/io.pyi

PYBIND11_MODULE(io, m_io)
{
    m_io.def("parse_dd_file", &mgm::io::parse_dd_file,
            py::arg("dd_file"),
            py::arg("unary_constant") = 0.0);
            
    m_io.def("parse_dd_file_gm", &mgm::io::parse_dd_file_gm,
            py::arg("dd_file"),
            py::arg("unary_constant") = 0.0);
            
    m_io.def("save_to_disk", 
            py::overload_cast<std::filesystem::path, const mgm::MgmSolution&>(&mgm::io::save_to_disk), 
            py::arg("filepath"), 
            py::arg("solution"));
            
    m_io.def("save_to_disk", 
            py::overload_cast<std::filesystem::path, const mgm::GmSolution&>(&mgm::io::save_to_disk), 
            py::arg("filepath"), 
            py::arg("solution"));
            
    m_io.def("export_dd_file", &mgm::io::export_dd_file, 
            py::arg("filepath"), 
            py::arg("model"));
            
    m_io.def("import_solution", &mgm::io::import_from_disk, 
            py::arg("solution_path"), 
            py::arg("model"));

    m_io.def("_register_io_logger", &register_python_logger);
}