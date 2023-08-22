#ifndef LIBMGM_CLIQUES_HPP
#define LIBMGM_CLIQUES_HPP

#include <vector>

#include "solution.hpp"
#include "multigraph.hpp"

class CliqueTable {
    private:
        typedef std::vector<int> Clique;

    public:
        CliqueTable(int no_graphs);
        CliqueTable(MgmSolution&);
        int no_graphs;
        int no_cliques;

        int& operator()(int clique_id, int graph_id);
        const int& operator()(int clique_id, int graph_id) const;

        void add_clique();

        MgmSolution export_solution(std::shared_ptr<MgmModel> model);
    private:
        std::vector<Clique> cliques;
        Clique empty_clique;

        void build_cliques(MgmSolution&);

};
#endif