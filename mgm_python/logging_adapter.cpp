#include <libmgm/mgm.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <iostream>
#include <queue>
#include <mutex>

#include "logging_adapter.hpp"

namespace py = pybind11;

/* LOGGER */
python_sink::python_sink(py::object py_logger) : py_logger_(py_logger) {}

void python_sink::sink_it_(const spdlog::details::log_msg& msg) {   
    PyThreadState* tstate = PyGILState_GetThisThreadState();
    if (!tstate || !PyGILState_Check()) {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        // Logging from C++ thread not created by python is invalid.
        int level   = spdlog_to_python_level(msg.level);
        auto msg_form = format_message(msg, thread_formatter);

        // Delay logging to later point.
        this->log_queue_.emplace(level, msg_form);
        return;
    }

    py::gil_scoped_acquire gil;  // Ensure python GIL is held
    this->clear_queue();

    // Call the Python logger with the appropriate level
    int lvl         = spdlog_to_python_level(msg.level);
    auto msg_form   = format_message(msg, default_formatter);

    py_log(lvl, msg_form);
}

void python_sink::flush_() {  
    PyThreadState* tstate = PyGILState_GetThisThreadState();
    if (!tstate || !PyGILState_Check()) {
        // Logging from C++ thread not created by python is invalid.
        return;
    }
    
    py::gil_scoped_acquire gil;  // Ensure python GIL is held
    this->clear_queue();
    py_logger_.attr("flush")();
}

std::string python_sink::format_message(const spdlog::details::log_msg& msg, const std::unique_ptr<spdlog::formatter>& formatter) {
    // Format the message using the formatter
    spdlog::memory_buf_t formatted;
    formatter->format(msg, formatted);

    // Strip trailing newlines
    std::string log_message = fmt::to_string(formatted);
    if (!log_message.empty() && log_message.back() == '\n') {
        log_message.pop_back();
    }

    return log_message;
}

void python_sink::clear_queue() {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    // Clear the queue if needed
    while (!log_queue_.empty()) {
        const auto& [py_lvl, py_msg] = log_queue_.front();
        
        py_log(py_lvl, py_msg);

        log_queue_.pop();
    }
}

void python_sink::py_log(int level, const std::string& msg) {
    py_logger_.attr("log")(level, py::str(msg));
}

int python_sink::spdlog_to_python_level(spdlog::level::level_enum level) {
    switch (level) {
        case spdlog::level::info:       return 20;  // Python's logging level for INFO
        case spdlog::level::debug:      return 10;  // Python's logging level for DEBUG
        case spdlog::level::warn:       return 30;  // Python's logging level for WARNING
        case spdlog::level::err:        return 40;  // Python's logging level for ERROR
        case spdlog::level::critical:   return 50;  // Python's logging level for CRITICAL
        case spdlog::level::trace:      return 5;  // Python's logging level for NOTSET
        case spdlog::level::off:        return 0;  // No logging
        default:                        return 20;  // Default to INFO
    }
}

void register_python_logger(py::object py_logger) {
    auto custom_sink = std::make_shared<python_sink>(py_logger);
    
    // Check if the logger already exists
    auto existing_logger = spdlog::get("libmgm");
    if (existing_logger) {
        // Add the custom sink to the existing logger
        existing_logger->sinks().push_back(custom_sink);
        return;
    }

    // Create a new logger if it doesn't exist
    auto python_logger = std::make_shared<spdlog::logger>("libmgm", custom_sink);
    python_logger->set_pattern("[LIBMGM] %v");
    spdlog::set_default_logger(python_logger);
}