#include <libmgm/mgm.hpp>
#include <filesystem>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

namespace py = pybind11;
namespace fs = std::filesystem;
using namespace mgm;

std::shared_ptr<MgmModel> parse_dd_file_python(fs::path dd_file) {
    return std::make_shared<MgmModel>(io::parse_dd_file(dd_file));
}

PYBIND11_MODULE(pylibmgm, m)
{   
    // mutigraph.hpp
    py::class_<Graph>(m, "Graph")
        .def(py::init<>())
        .def(py::init<int, int>())
        .def_readwrite("id", &Graph::id)
        .def_readwrite("no_nodes", &Graph::no_nodes);

    py::class_<GmModel, std::shared_ptr<GmModel>>(m, "GmModel")
        .def("add_assignment", &GmModel::add_assignment)
        .def("add_edge", py::overload_cast<int, int, double>(&GmModel::add_edge), "Add an edge via two assignment ids")
        .def("add_edge", py::overload_cast<int, int, int, int, double>(&GmModel::add_edge), "Add an edge via four node ids")
        .def_readwrite("graph1", &GmModel::graph1)
        .def_readwrite("graph2", &GmModel::graph2)
        .def_readwrite("no_assignments", &GmModel::no_assignments)
        .def_readwrite("no_edges", &GmModel::no_edges)
        .def_readwrite("no_edges", &GmModel::no_edges);

    py::class_<MgmModel, std::shared_ptr<MgmModel>>(m, "MgmModel")
        .def_readwrite("no_graphs", &MgmModel::no_graphs)
        .def_readwrite("graphs", &MgmModel::graphs)
        .def_readwrite("models", &MgmModel::models);

    // solution.hpp
    py::class_<GmSolution>(m, "GmSolution")
        .def(py::init<>())
        .def(py::init<std::shared_ptr<GmModel>>())
        .def("evaluate", &GmSolution::evaluate)
        .def_readwrite("labeling", &GmSolution::labeling);

    py::class_<MgmSolution>(m, "MgmSolution")
        .def(py::init<std::shared_ptr<MgmModel>>());

    // solver_mgm.hpp
    py::class_<MgmGenerator, std::unique_ptr<MgmGenerator, py::nodelete>>(m, "MgmGenerator")
        .def("export_solution", &MgmGenerator::export_solution);

    py::class_<SequentialGenerator> SeqGen(m, "SequentialGenerator");
        SeqGen.def(py::init<std::shared_ptr<MgmModel>>());
        SeqGen.def("generate", &SequentialGenerator::generate);
        SeqGen.def("init_generation_sequence", &SequentialGenerator::init_generation_sequence);

    py::enum_<SequentialGenerator::matching_order>(SeqGen, "matching_order")
        .value("sequential",    SequentialGenerator::matching_order::sequential)
        .value("random",        SequentialGenerator::matching_order::random)
        .export_values();
        
    auto m_io = m.def_submodule("io", "Input/Output utilities");
    m_io.def("parse_dd_file", &parse_dd_file_python);
}