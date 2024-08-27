#include <libmgm/mgm.hpp>

#include <filesystem>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>

namespace py = pybind11;

/* LOGGER 
Necessary to register default python loggers with spdlog, which is used in c++ code.
*/
class python_sink : public spdlog::sinks::base_sink<std::mutex> {
    public:
        python_sink(py::object py_logger);

    protected:
        void sink_it_(const spdlog::details::log_msg& msg) override;
        void flush_() override;

    private:
        py::object py_logger_;
        int spdlog_to_python_level(spdlog::level::level_enum level);
};

void register_python_logger(py::object py_logger);