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

class CliqueManager {
    public:
        CliqueManager() = default;
        CliqueManager(Graph g);
        CliqueManager(std::vector<int> graph_ids, const MgmModel& model);
        CliqueManager(std::vector<int> graph_ids, const MgmModel& model, CliqueTable table);

        // (clique_id, graph_id) -> node_id;
        CliqueTable cliques;
        
        std::vector<int> graph_ids;

        const int& clique_idx(int graph_id, int node_id) const;

        void build_clique_idx_view();
        void remove_graph(int graph_id, bool should_prune=true);
        void prune();
        
    private:
        int& clique_idx(int graph_id, int node_id);

        // Stores idx of clique in CliqueTable for every node in a graph.
        // [graph_id][node_id] -> clique_idx;
        std::unordered_map<int, std::vector<int>> clique_idx_view;
};

}
#endif