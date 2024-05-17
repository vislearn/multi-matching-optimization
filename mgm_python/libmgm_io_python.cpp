#include <libmgm/mgm.hpp>

#include <filesystem>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl/filesystem.h>

namespace py = pybind11;
namespace fs = std::filesystem;

std::shared_ptr<mgm::MgmModel> parse_dd_file_python(fs::path dd_file, double unary_constant) {
    return std::make_shared<mgm::MgmModel>(mgm::io::parse_dd_file(dd_file, unary_constant));
}


/* LOGGER */
class python_sink : public spdlog::sinks::base_sink<std::mutex> {
public:
    python_sink(py::object py_logger) : py_logger_(py_logger) {}

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override {
        // Format the message using the formatter
        spdlog::memory_buf_t formatted;
        base_sink<std::mutex>::formatter_->format(msg, formatted);

        // Convert to a Python string
        py::str py_msg = py::str(fmt::to_string(formatted));

        // Call the Python logger with the appropriate level
        py_logger_.attr("log")(msg.level, py_msg);
    }

    void flush_() override {
        py_logger_.attr("flush")();
    }

private:
    py::object py_logger_;
};

void register_python_logger(py::object py_logger) {
    auto custom_sink = std::make_shared<python_sink>(py_logger);
    auto logger = std::make_shared<spdlog::logger>("python_logger", custom_sink);
    spdlog::register_logger(logger);
}

PYBIND11_MODULE(io, m_io)
{   
    m_io.def("parse_dd_file", &parse_dd_file_python,
            py::arg("dd_file"),
            py::arg("unary_constant") = 0.0);

    m_io.def("safe_to_disk", &mgm::io::safe_to_disk);
    m_io.def("export_dd_file", &mgm::io::export_dd_file);
    m_io.def("register_logger", &register_python_logger, "Register a Python logger with spdlog");
}