#include <libmgm/mgm.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <omp.h>

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

py::list labeling_to_list(const std::vector<int>& labeling) {
    py::list converted_list;

    for (int x : labeling) {
        if (x == -1) {
            converted_list.append(py::none());
        } else {
            converted_list.append(x);
        }
    }
    return converted_list;
}

py::list gm_solution_to_list_with_none(const GmSolution &solution) {
    return labeling_to_list(solution.labeling());
}

py::dict mgm_solution_to_dict_with_none(const MgmSolution &solution) {
    py::dict converted_labeling;

    for (const auto& [key, gm_labeling] : solution.labeling()) {
        py::list converted_list = labeling_to_list(gm_labeling);
        converted_labeling[py::cast(key)] = converted_list;
    }
    return converted_labeling;
}

PYBIND11_MODULE(_pylibmgm, m)
{   
    // costs.hpp
    py::class_<CostMap>(m, "CostMap")
        .def("unary",       py::overload_cast<int, int>(&CostMap::unary, py::const_))
        .def("unary",       py::overload_cast<AssignmentIdx>(&CostMap::unary, py::const_))
        .def("pairwise",    py::overload_cast<int, int, int, int>(&CostMap::pairwise, py::const_))
        .def("pairwise",    py::overload_cast<EdgeIdx>(&CostMap::pairwise, py::const_))
        .def("contains",    py::overload_cast<int, int>(&CostMap::unary, py::const_))
        .def("contains",    py::overload_cast<AssignmentIdx>(&CostMap::unary, py::const_))
        .def("contains",    py::overload_cast<int, int, int, int>(&CostMap::pairwise, py::const_))
        .def("contains",    py::overload_cast<EdgeIdx>(&CostMap::pairwise, py::const_))
        .attr("__module__") = "pylibmgm";

    // mutigraph.hpp
    py::class_<Graph>(m, "Graph")
        .def(py::init<int, int>())
        .def_readwrite("id", &Graph::id)
        .def_readwrite("no_nodes", &Graph::no_nodes)
        .attr("__module__") = "pylibmgm";

    py::class_<GmModel, std::shared_ptr<GmModel>>(m, "GmModel")
        .def(py::init<Graph, Graph>())
        .def(py::init<Graph, Graph, int, int>())
        .def("add_assignment", &GmModel::add_assignment)
        .def("add_edge", py::overload_cast<int, int, double>(&GmModel::add_edge), "Add an edge via two assignment ids")
        .def("add_edge", py::overload_cast<int, int, int, int, double>(&GmModel::add_edge), "Add an edge via four node ids")
        .def("no_assignments", &GmModel::no_assignments)
        .def("no_edges", &GmModel::no_edges)
        .def_readonly("assignment_list", &GmModel::assignment_list)
        .def("costs", [](GmModel& self) { return self.costs.get(); }, py::return_value_policy::reference_internal)
        .def_readwrite("graph1", &GmModel::graph1)
        .def_readwrite("graph2", &GmModel::graph2)
        .attr("__module__") = "pylibmgm";

    py::class_<MgmModel, std::shared_ptr<MgmModel>>(m, "MgmModel")
        .def(py::init<>())
        .def_readwrite("no_graphs", &MgmModel::no_graphs)
        .def_readwrite("graphs", &MgmModel::graphs)
        .def_readwrite("models", &MgmModel::models)    
        .def("create_submodel", &MgmModel::create_submodel)  
        .def("add_model", &mgm_model_add_model)
        .attr("__module__") = "pylibmgm";

    // solution.hpp
    py::class_<GmSolution>(m, "GmSolution")
        .def(py::init<>())
        .def(py::init<std::shared_ptr<GmModel>>())
        .def(py::init<std::shared_ptr<GmModel>, std::vector<int>>())
        .def_static("evaluate_static", py::overload_cast<const GmModel&, const std::vector<int>& >(&GmSolution::evaluate))
        .def("evaluate", py::overload_cast<>(&GmSolution::evaluate, py::const_))
        .def("to_list_with_none", &gm_solution_to_list_with_none)
        .def("labeling", py::overload_cast<>(&GmSolution::labeling))
        .def("__getitem__", [](const GmSolution &sol, int idx) {
                if((size_t) idx >= sol.labeling().size()) {
                    throw py::index_error();
                }
                return sol[idx];
            })
        .def("__setitem__", [](GmSolution &self, int index, int val)
                    { self[index] = val; })
        .attr("__module__") = "pylibmgm";

    py::class_<MgmSolution>(m, "MgmSolution")
        .def(py::init<std::shared_ptr<MgmModel>>())
        .def("evaluate", py::overload_cast<>(&MgmSolution::evaluate, py::const_))
        .def("evaluate", py::overload_cast<int>(&MgmSolution::evaluate, py::const_))
        .def("labeling",        &MgmSolution::labeling, py::return_value_policy::reference)
        .def("to_dict_with_none", &mgm_solution_to_dict_with_none)
        .def("set_solution", py::overload_cast<const Labeling&>(&MgmSolution::set_solution))
        .def("set_solution", py::overload_cast<const GmModelIdx& , std::vector<int> >(&MgmSolution::set_solution))
        .def("set_solution", py::overload_cast<const GmSolution&>(&MgmSolution::set_solution))
        .def("create_empty_labeling", &MgmSolution::create_empty_labeling)
        .def_readwrite("model", &MgmSolution::model)
        .def("__getitem__", py::overload_cast<GmModelIdx>(&MgmSolution::operator[], py::const_),
                            py::return_value_policy::reference)
        .def("__setitem__", [](MgmSolution &self, const GmModelIdx& index, std::vector<int> labeling)
                            { self.set_solution(index, labeling);})
        .def("__len__", [](const MgmSolution &self) 
                            { return self.labeling().size(); })
        .attr("__module__") = "pylibmgm";

    // solver_generator_mgm.hpp
    py::class_<MgmGenerator, std::unique_ptr<MgmGenerator, py::nodelete>> MgmGen(m, "MgmGenerator");
    MgmGen.attr("__module__") = "pylibmgm";

    py::enum_<MgmGenerator::matching_order>(MgmGen, "matching_order")
        .value("sequential",    SequentialGenerator::matching_order::sequential)
        .value("random",        SequentialGenerator::matching_order::random)
        .export_values();

    py::class_<SequentialGenerator, MgmGenerator> (m, "SequentialGenerator")
        .def(py::init<std::shared_ptr<MgmModel>>())
        .def("init",        &SequentialGenerator::init)
        .def("generate",    &SequentialGenerator::generate)
        .def("step",        &SequentialGenerator::step)
        .attr("__module__") = "pylibmgm";


    py::class_<ParallelGenerator, MgmGenerator>(m, "ParallelGenerator")
        .def(py::init<std::shared_ptr<MgmModel>>())
        .def("init",        &ParallelGenerator::init)
        .def("generate", &ParallelGenerator::generate)
        .attr("__module__") = "pylibmgm";


    // solver_local_search_GM.hpp
    py::class_<GMLocalSearcher>(m, "GMLocalSearcher")
        .def(py::init<std::shared_ptr<MgmModel>>())
        .def(py::init<std::shared_ptr<MgmModel>, std::vector<int>>())
        .def("search", [](GMLocalSearcher &self, MgmSolution &input) {
            return self.search(input);
        })
        .attr("__module__") = "pylibmgm";

    py::class_<GMLocalSearcherParallel>(m, "GMLocalSearcherParallel")
        .def(py::init<std::shared_ptr<MgmModel>, bool>(), 
            py::arg("model"),
            py::arg("merge_all") = true)        
        .def("search", [](GMLocalSearcherParallel &self, MgmSolution &input) {
            return self.search(input);
        })
        .attr("__module__") = "pylibmgm";

    // qap_interface.hpp
    py::class_<QAPSolver>(m, "QAPSolver")
        .def(py::init<std::shared_ptr<GmModel>, int, int>(),
            py::arg("model"),
            py::arg("batch_size") = 10, 
            py::arg("greedy_generations") = 10)
        .def("run", &QAPSolver::run,
            py::arg("verbose") = false)
        .attr("__module__") = "pylibmgm";
    
    // lap_interface.hpp
    py::class_<LAPSolver>(m, "LAPSolver")
        .def(py::init<std::shared_ptr<GmModel>>())
        .def("run", &LAPSolver::run)
        .attr("__module__") = "pylibmgm";
    
    // solver_local_search_swap.hpp
    py::class_<SwapLocalSearcher>(m, "SwapLocalSearcher")
        .def(py::init<std::shared_ptr<MgmModel>>())
        .def("search", [](SwapLocalSearcher &self, MgmSolution &input) {
            return self.search(input);
        })
        .attr("__module__") = "pylibmgm";

    m.def("build_sync_problem", &mgm::build_sync_problem)
        .attr("__module__") = "pylibmgm";
    m.def("omp_set_num_threads", &omp_set_num_threads)
        .attr("__module__") = "pylibmgm";
    
    m.def("_register_api_logger", &register_python_logger, "Register a Python logger with spdlog");
}