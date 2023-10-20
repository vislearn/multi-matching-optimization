#ifndef LIBMGM_CLIQUES_HPP
#define LIBMGM_CLIQUES_HPP

#include <unordered_map>
#include <vector>
#include <ankerl/unordered_dense.h>

#include "multigraph.hpp"

namespace mgm {

class CliqueTable {
    public:
        // [graph_id] -> node_id
        typedef ankerl::unordered_dense::map<int, int> Clique;
        typedef std::vector<Clique>::iterator iterator;
        typedef std::vector<Clique>::const_iterator const_iterator;

        CliqueTable() = default;
        CliqueTable(int no_graphs);

        int no_graphs = 0;
        int no_cliques = 0; //TODO: Replace with .size() {return this->cliques.size();}

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
        void remove_graph(int graph_id, bool should_prune=true);
        void prune();

    private:
        std::vector<Clique> cliques;
        Clique empty_clique;
};
}
#endif