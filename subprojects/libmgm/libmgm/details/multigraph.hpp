#ifndef LIBMGM_MULTIGRAPH_HPP
#define LIBMGM_MULTIGRAPH_HPP

#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <utility>
#include <sqlite3.h>

#include "costs.hpp"

namespace mgm {

typedef std::pair<int,int> GmModelIdx;

struct GmModelIdxHash {
    std::size_t operator()(GmModelIdx const& input) const noexcept {
        size_t seed = 0;
        boost_hash_combine(seed, input.first);
        boost_hash_combine(seed, input.second);
        return seed;
    }
};

class Graph {
    public:
        Graph() {};
        Graph(int id, int no_nodes);

        int id;
        int no_nodes;
};

class GmModel{
    public:
        GmModel() {};
        GmModel(Graph g1, Graph g2, int no_assignments, int no_edges);
        Graph graph1;
        Graph graph2;

        int no_assignments;
        int no_edges;

        void add_assignment(int assignment_id, int node1, int node2, double cost);

        // both valid alternatives.
        void add_edge(int assignment1, int assigment2, double cost);
        void add_edge(int assignment1_node1, int assignment1_node2, int assignment2_node1, int assignment2_node2, double cost);

        std::vector<AssignmentIdx> assignment_list;
        std::vector<std::vector<int>> assignments_left;
        std::vector<std::vector<int>> assignments_right;
        std::unique_ptr<CostMap> costs;

        template <class Archive>
        void serialize(Archive& archive) {
            archive(assignment_list, assignments_left, assignments_right, costs);
        }
        void serialize_to_binary(std::string& result_string) const;
        void deserialize_from_binary(std::string& serialized_model);
        
};

class UnorderedMapWithCaches {
public:

private: 
    std::unordered_map<GmModelIdx, std::shared_ptr<GmModel>, GmModelIdxHash> cache_1;
    std::unordered_map<GmModelIdx, std::shared_ptr<GmModel>, GmModelIdxHash> cache_2;
};

class MgmModel {
    public:
        MgmModel();

        int no_graphs;
        std::vector<Graph> graphs;
        
        std::unordered_map<GmModelIdx, std::shared_ptr<GmModel>, GmModelIdxHash> models;
};

class SqlMgmModel: public MgmModel {
    public:
        SqlMgmModel();
        ~SqlMgmModel();

        SqlMgmModel(const SqlMgmModel& other) = default;             // Copy constructor maybe need multithreading flag for this
        SqlMgmModel& operator=(const SqlMgmModel& other) = default;  // Copy assignment operator
        SqlMgmModel(SqlMgmModel&& other);         // Move constructor
        SqlMgmModel& operator=(SqlMgmModel&& other);
        void save_model_to_db(const GmModel& gm_model, const GmModelIdx& idx);
        std::shared_ptr<GmModel> read_model_from_db(const GmModelIdx& idx);
    private:
        sqlite3* open_db();
        void create_table();
        void set_up_write_statement();
        void set_up_read_statement();
        void delete_table();
        void deserialize_serialized_model(std::string& serialized_model, GmModel& model);
        
        

        sqlite3* db;
        sqlite3_stmt* insert_stmt;
        sqlite3_stmt* read_stmt;
};
}
#endif