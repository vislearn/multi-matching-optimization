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

std::shared_ptr<mgm::MgmModel> parse_dd_file_python(fs::path dd_file, double unary_constant) {
    return std::make_shared<mgm::MgmModel>(mgm::io::parse_dd_file(dd_file, unary_constant));
}

PYBIND11_MODULE(io, m_io)
{   
    m_io.def("parse_dd_file", &parse_dd_file_python,
            py::arg("dd_file"),
            py::arg("unary_constant") = 0.0);

    m_io.def("safe_to_disk", &mgm::io::safe_to_disk);
    m_io.def("export_dd_file", &mgm::io::export_dd_file);
    m_io.def("import_solution", &mgm::io::import_from_disk);

    m_io.def("register_io_logger", &register_python_logger, "Register a Python logger with spdlog");
}