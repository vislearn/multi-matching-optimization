#include <libmgm/mgm.hpp>
#include <filesystem>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

namespace py = pybind11;
namespace fs = std::filesystem;
using namespace mgm;

std::shared_ptr<MgmModel> parse_dd_file_python(fs::path dd_file, double unary_constant) {
    return std::make_shared<MgmModel>(io::parse_dd_file(dd_file, unary_constant));
}

PYBIND11_MODULE(_pylibmgm, m)
{   
    // mutigraph.hpp
    py::class_<Graph>(m, "Graph")
        .def(py::init<int, int>())
        .def_readwrite("id", &Graph::id)
        .def_readwrite("no_nodes", &Graph::no_nodes);

    py::class_<GmModel, std::shared_ptr<GmModel>>(m, "GmModel")
        .def(py::init<Graph, Graph>())
        .def(py::init<Graph, Graph, int, int>())
        .def("add_assignment", &GmModel::add_assignment)
        .def("add_edge", py::overload_cast<int, int, double>(&GmModel::add_edge), "Add an edge via two assignment ids")
        .def("add_edge", py::overload_cast<int, int, int, int, double>(&GmModel::add_edge), "Add an edge via four node ids")
        .def("no_assignments", &GmModel::no_assignments)
        .def("no_edges", &GmModel::no_edges)
        .def_readwrite("graph1", &GmModel::graph1)
        .def_readwrite("graph2", &GmModel::graph2);

    py::class_<MgmModel, std::shared_ptr<MgmModel>>(m, "MgmModel")
        .def_readwrite("no_graphs", &MgmModel::no_graphs)
        .def_readwrite("graphs", &MgmModel::graphs)
        .def_readwrite("models", &MgmModel::models);

    // // costs.hpp
    // py::class_<CostMap>(m, "CostMap")
    //     .def("set_unary", &CostMap::set_unary)        
    //     .def("set_pairwise", &CostMap::set_pairwise);

    // solution.hpp
    py::class_<GmSolution>(m, "GmSolution")
        .def(py::init<>())
        .def(py::init<std::shared_ptr<GmModel>>())
        .def("evaluate", &GmSolution::evaluate)
        .def_readwrite("labeling", &GmSolution::labeling, py::return_value_policy::reference);

    py::class_<MgmSolution>(m, "MgmSolution")
        .def(py::init<std::shared_ptr<MgmModel>>())
        .def("evaluate", &MgmSolution::evaluate)
        .def_readwrite("gmSolutions", &MgmSolution::gmSolutions)
        .def("__getitem__", py::overload_cast<GmModelIdx>(&MgmSolution::operator[], py::const_),
                            py::return_value_policy::reference)
        .def("__setitem__", [](MgmSolution &self, GmModelIdx index, GmSolution val)
                    { self[index] = val; });

    // cliques.hpp
    py::class_<CliqueTable>(m, "CliqueTable")
        .def_readonly("no_cliques", &CliqueTable::no_cliques);

    // solver_mgm.hpp
    py::class_<MgmGenerator, std::unique_ptr<MgmGenerator, py::nodelete>>(m, "MgmGenerator")
        .def("export_solution", &MgmGenerator::export_solution)
        .def("export_cliquemanager", &MgmGenerator::export_CliqueManager);

    py::class_<SequentialGenerator, MgmGenerator> SeqGen(m, "SequentialGenerator");
        SeqGen.def(py::init<std::shared_ptr<MgmModel>>());
        SeqGen.def("generate", &SequentialGenerator::generate);
        SeqGen.def("init_generation_sequence", &SequentialGenerator::init_generation_sequence);

    py::enum_<SequentialGenerator::matching_order>(SeqGen, "matching_order")
        .value("sequential",    SequentialGenerator::matching_order::sequential)
        .value("random",        SequentialGenerator::matching_order::random)
        .export_values();

    py::class_<CliqueManager>(m, "CliqueManager")
        .def("reconstruct_from", &CliqueManager::reconstruct_from)
        .def_readonly("cliques", &CliqueManager::cliques);

    // solver_local_search.hpp
    py::class_<LocalSearcher>(m, "LocalSearcher")
        .def(py::init<CliqueManager, std::shared_ptr<MgmModel>>())
        .def(py::init<CliqueManager, std::vector<int>, std::shared_ptr<MgmModel>>())
        .def("export_solution", &LocalSearcher::export_solution)
        .def("export_cliquemanager", &LocalSearcher::export_CliqueManager)
        .def("export_cliquetable", &LocalSearcher::export_cliquetable)
        .def("search", &LocalSearcher::search);

    // qap_interface.hpp
    py::class_<QAPSolver>(m, "QAPSolver")
        .def(py::init<std::shared_ptr<GmModel>, int, int, int>(),
            py::arg("model"),
            py::arg("batch_size") = 10, 
            py::arg("max_batches") = 10, 
            py::arg("greedy_generations") = 10)
        .def("run", &QAPSolver::run,
            py::arg("verbose") = false);
    
    // qap_interface.hpp
    py::class_<ABOptimizer>(m, "ABOptimizer")
        .def(py::init<CliqueTable, std::shared_ptr<MgmModel>>())
        .def("search", &ABOptimizer::search)
        .def("export_solution", &ABOptimizer::export_solution)
        .def("export_cliquetable", &ABOptimizer::export_cliquetable);


    auto m_io = m.def_submodule("io", "Input/Output utilities");
    m_io.def("parse_dd_file", &parse_dd_file_python,
            py::arg("dd_file"),
            py::arg("unary_constant") = 0.0);

    m_io.def("safe_to_disk", &io::safe_to_disk);
}