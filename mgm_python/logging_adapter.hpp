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

        const std::unique_ptr<spdlog::formatter> default_formatter  = std::make_unique<spdlog::pattern_formatter>("[LIBMGM] %v");
        const std::unique_ptr<spdlog::formatter> thread_formatter   = std::make_unique<spdlog::pattern_formatter>("[LIBMGM] [t_id %t] [async: %Y-%m-%d %H:%M:%S,%e] %v");

        std::mutex queue_mutex_;
        std::queue<std::pair<int, std::string>> log_queue_; // <level, message> pairs.

        void clear_queue();
        void py_log(int level, const std::string &msg);
        
        std::string format_message(const spdlog::details::log_msg &msg, const std::unique_ptr<spdlog::formatter> &formatter);
        int spdlog_to_python_level(spdlog::level::level_enum level);
};

void register_python_logger(py::object py_logger);