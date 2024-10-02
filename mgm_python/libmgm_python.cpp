#include <libmgm/mgm.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "logging_adapter.hpp"

namespace py = pybind11;
using namespace mgm;

// TODO: make this a member function. Could be useful for core code as well.
void mgm_model_add_model(MgmModel& mgm_model, std::shared_ptr<GmModel> gm_model) {
    int g1 = gm_model->graph1.id;
    int g2 = gm_model->graph2.id;

    GmModelIdx idx(g1, g2);

    mgm_model.models[idx] = gm_model;

    if (g2 >= mgm_model.no_graphs) {
        mgm_model.no_graphs = g2 + 1;
        mgm_model.graphs.resize(g2 + 1);
    }

    // TODO: This should be sanitiy checked
    mgm_model.graphs[g1] = gm_model->graph1;
    mgm_model.graphs[g2] = gm_model->graph2;
}

// Define the to_dict function for MgmSolution
py::list gm_solution_to_list(const GmSolution &solution) {
    py::list converted_list;

    for (int x : solution.labeling) {
        if (x == -1) {
            converted_list.append(py::none());
        } else {
            converted_list.append(x);
        }
    }
    return converted_list;
}

// Define the to_dict function for MgmSolution
py::dict mgm_solution_to_dict(const MgmSolution &solution) {
    py::dict dict;
    for (const auto& [key, solution] : solution.gmSolutions) {
        py::tuple pykey = py::make_tuple(key.first, key.second);
        py::list converted_list = gm_solution_to_list(solution);
        
        dict[pykey] = converted_list;
    }
    return dict;
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
        .def_readonly("assignment_list", &GmModel::assignment_list)
        .def_readwrite("graph1", &GmModel::graph1)
        .def_readwrite("graph2", &GmModel::graph2);

    py::class_<MgmModel, std::shared_ptr<MgmModel>>(m, "MgmModel")
        .def(py::init<>())
        .def_readwrite("no_graphs", &MgmModel::no_graphs)
        .def_readwrite("graphs", &MgmModel::graphs)
        .def_readwrite("models", &MgmModel::models)   
        .def("add_model", &mgm_model_add_model);

    // solution.hpp
    py::class_<GmSolution>(m, "GmSolution")
        .def(py::init<>())
        .def(py::init<std::shared_ptr<GmModel>>())
        .def("evaluate", &GmSolution::evaluate)
        .def("to_list", &gm_solution_to_list)
        .def_readwrite("labeling", &GmSolution::labeling, py::return_value_policy::reference)
        .def("__getitem__", [](const GmSolution &sol, int idx) {
                if((size_t) idx >= sol.labeling.size()) {
                    throw py::index_error();
                }
                return sol[idx];
            })
        .def("__setitem__", [](GmSolution &self, int index, int val)
                    { self[index] = val; });

    py::class_<MgmSolution>(m, "MgmSolution")
        .def(py::init<std::shared_ptr<MgmModel>>())
        .def("evaluate", py::overload_cast<>(&MgmSolution::evaluate, py::const_))
        .def("to_dict", &mgm_solution_to_dict)
        .def_readwrite("gmSolutions", &MgmSolution::gmSolutions)
        .def_readwrite("model", &MgmSolution::model)
        .def("__getitem__", py::overload_cast<GmModelIdx>(&MgmSolution::operator[], py::const_),
                            py::return_value_policy::reference)
        .def("__setitem__", [](MgmSolution &self, GmModelIdx index, GmSolution val)
                    { self[index] = val; })
        .def("__len__", [](const MgmSolution &self) {
            return self.gmSolutions.size();
        })
        .def("__iter__", [](const MgmSolution &self) {
            return py::make_iterator(self.gmSolutions.begin(), self.gmSolutions.end());
        }, py::keep_alive<0, 1>());  // Keep the self object alive while the iterator is used

    // cliques.hpp
    py::class_<CliqueTable>(m, "CliqueTable")
        .def_readonly("no_cliques", &CliqueTable::no_cliques);

    // solver_mgm.hpp
    py::class_<MgmGenerator, std::unique_ptr<MgmGenerator, py::nodelete>>(m, "MgmGenerator")
        .def("export_solution", &MgmGenerator::export_solution)
        .def("export_cliquemanager", &MgmGenerator::export_CliqueManager);

    py::class_<SequentialGenerator, MgmGenerator> SeqGen(m, "SequentialGenerator");
        SeqGen.def(py::init<std::shared_ptr<MgmModel>>());
        SeqGen.def("init", &SequentialGenerator::init);
        SeqGen.def("generate", &SequentialGenerator::generate);
        SeqGen.def("step", &SequentialGenerator::step);
        SeqGen.def("set_state", &SequentialGenerator::set_state);

    py::enum_<SequentialGenerator::matching_order>(SeqGen, "matching_order")
        .value("sequential",    SequentialGenerator::matching_order::sequential)
        .value("random",        SequentialGenerator::matching_order::random)
        .export_values();

    py::class_<CliqueManager>(m, "CliqueManager")
        .def("reconstruct_from", &CliqueManager::reconstruct_from)
        .def_readonly("cliques", &CliqueManager::cliques)
        .def_readonly("graph_ids", &CliqueManager::graph_ids);

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
    
    // lap_interface.hpp
    py::class_<LAPSolver>(m, "LAPSolver")
        .def(py::init<std::shared_ptr<GmModel>>())
        .def("run", &LAPSolver::run);
    
    // qap_interface.hpp
    py::class_<ABOptimizer>(m, "ABOptimizer")
        .def(py::init<CliqueTable, std::shared_ptr<MgmModel>>())
        .def("search", &ABOptimizer::search)
        .def("export_solution", &ABOptimizer::export_solution)
        .def("export_cliquetable", &ABOptimizer::export_cliquetable);

    m.def("build_sync_problem", &mgm::build_sync_problem);
    
    m.def("_register_api_logger", &register_python_logger, "Register a Python logger with spdlog");
}