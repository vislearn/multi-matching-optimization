#include <vector>

#include "cliques.hpp"
#include "multigraph.hpp"

CliqueTable::CliqueTable(int no_graphs) {
    this->no_graphs = no_graphs;
    this->empty_clique = std::vector<int>(this->no_graphs, -1);
}

CliqueTable::CliqueTable(MgmSolution& solution) {
    this->no_graphs = solution.model->no_graphs;
    this->empty_clique = std::vector<int>(this->no_graphs, -1);

    this->build_cliques(solution);
}

void CliqueTable::add_clique() {
    this->cliques.push_back(empty_clique);
    this->no_cliques++;
}

int& CliqueTable::operator()(int clique_id, int graph_id) {
    return this->cliques.at(clique_id).at(graph_id);
}

const int& CliqueTable::operator()(int clique_id, int graph_id) const {
    return this->cliques.at(clique_id).at(graph_id);
}

MgmSolution CliqueTable::export_solution(std::shared_ptr<MgmModel> model) {
    return MgmSolution(model);
}

void CliqueTable::build_cliques(MgmSolution& solution) {
    this->add_clique();
}
