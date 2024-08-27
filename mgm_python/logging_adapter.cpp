#include <libmgm/mgm.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>

namespace py = pybind11;

/* LOGGER */
class python_sink : public spdlog::sinks::base_sink<std::mutex> {
public:
    python_sink(py::object py_logger) : py_logger_(py_logger) {}

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override {
        // Format the message using the formatter
        spdlog::memory_buf_t formatted;
        base_sink<std::mutex>::formatter_->format(msg, formatted);

        // Convert the message to a Python string and strip trailing newlines
        std::string log_message = fmt::to_string(msg.payload);
        if (!log_message.empty() && log_message.back() == '\n') {
            log_message.pop_back();
        }
        log_message = "[LIBMGM] " + log_message;

        // Call the Python logger with the appropriate level
        int py_level = spdlog_to_python_level(msg.level);
        py::object py_msg = py::str(log_message);

        py_logger_.attr("log")(py_level, py_msg);
    }

    void flush_() override {
        py_logger_.attr("flush")();
    }

private:
    py::object py_logger_;
    int spdlog_to_python_level(spdlog::level::level_enum level) {
        switch (level) {
            case spdlog::level::info:
                return 20;  // Python's logging level for INFO
            case spdlog::level::debug:
                return 10;  // Python's logging level for DEBUG
            case spdlog::level::warn:
                return 30;  // Python's logging level for WARNING
            case spdlog::level::err:
                return 40;  // Python's logging level for ERROR
            case spdlog::level::critical:
                return 50;  // Python's logging level for CRITICAL
            case spdlog::level::trace:
                return 5;  // Python's logging level for NOTSET
            case spdlog::level::off:
                return 0;  // No logging
            default:
                return 20;  // Default to INFO
        }
    }

};

void register_python_logger(py::object py_logger) {
    auto custom_sink = std::make_shared<python_sink>(py_logger);
    auto python_logger = std::make_shared<spdlog::logger>("libmgm", custom_sink);
    python_logger->set_pattern("%v");
    spdlog::set_default_logger(python_logger);
}