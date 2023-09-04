#ifndef LIBMGM_CLIQUES_HPP
#define LIBMGM_CLIQUES_HPP

#include <unordered_map>
#include <vector>

#include "solution.hpp"
#include "multigraph.hpp"

namespace mgm {

class CliqueTable {
    public:
        // [graph_id] -> node_id
        typedef std::unordered_map<int, int> Clique;
        typedef std::vector<std::unordered_map<int, int>>::iterator iterator;
        typedef std::vector<std::unordered_map<int, int>>::const_iterator const_iterator;

        CliqueTable(int no_graphs);
        CliqueTable(MgmSolution&);

        int no_graphs = 0;
        int no_cliques = 0;

        int& operator()(int clique_id, int graph_id);
        const int& operator()(int clique_id, int graph_id) const;

        Clique& operator[](int clique_id);
        const Clique& operator[](int clique_id) const;

        iterator begin() { return this->cliques.begin();}
        iterator end() { return this->cliques.end();}
        const_iterator begin() const { return this->cliques.begin();}
        const_iterator end() const { return this->cliques.end();}

        void add_clique();
        void add_clique(Clique c);
        void reserve(int no_cliques);

        MgmSolution export_solution(std::shared_ptr<MgmModel> model);

    private:
        std::vector<Clique> cliques;
        Clique empty_clique;

        void build_cliques(MgmSolution&);
};
}
#endif