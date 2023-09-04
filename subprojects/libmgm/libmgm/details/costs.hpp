#ifndef LIBMGM_COSTS_HPP
#define LIBMGM_COSTS_HPP

#include <unordered_map>
#include <utility>

namespace mgm {
/*
* FIXME: Using boost hash combine improved performance drastically. 
*   However, this could probably be improved with a more specialize Hashtable for large number of entries (number of edges ~500.000).
*/
typedef std::pair<int,int> AssignmentIdx;
typedef std::pair<AssignmentIdx, AssignmentIdx> EdgeIdx;


void boost_hash_combine(size_t& seed, const int& v);

struct AssignmentIdxHash {
    std::size_t operator()(AssignmentIdx const& input) const noexcept {
        size_t seed = 0;
        boost_hash_combine(seed, input.first);
        boost_hash_combine(seed, input.second);
        return seed;
    }
};

struct EdgeIdxHash {
    std::size_t operator()(EdgeIdx const& input) const noexcept {
        size_t seed = 0;
        boost_hash_combine(seed, input.first.first);
        boost_hash_combine(seed, input.first.second);
        boost_hash_combine(seed, input.second.first);
        boost_hash_combine(seed, input.second.second);
        return seed;
    }
};

typedef std::unordered_map<AssignmentIdx, double, AssignmentIdxHash> AssignmentContainer;
typedef std::unordered_map<EdgeIdx, double, EdgeIdxHash> EdgeContainer;

// class ICostStructure {
//     public:
//         virtual ~ICostStructure() = 0;
        
//         virtual const double& unary(int node1, int node2) = 0;
//         virtual const double& pairwise(int node1, int node2, int node3, int node4) = 0;

//         const double& unary(AssignmentIdx assignment);
//         const double& pairwise(EdgeIdx edge);
        
//         virtual void set_unary(int node1, int node2, double cost) = 0;
//         virtual void set_pairwise(int node1, int node2, int node3, int node4, double cost) = 0;

//     protected:
//         // rule of five
//         ICostStructure()                                        = default;
//         ICostStructure(const ICostStructure& other)             = default;
//         ICostStructure(ICostStructure&& other)                  = default;
//         ICostStructure& operator=(const ICostStructure& other)  = default;
//         ICostStructure& operator=(ICostStructure&& other)       = default;
// };

class CostMap {
    public:
        CostMap(int no_unaries, int no_pairwise);
        ~CostMap() {};
        
        const double& unary(int node1, int node2)                           const;
        const double& unary(AssignmentIdx assignment)                       const;
        
        const double& pairwise(int node1, int node2, int node3, int node4)  const;
        const double& pairwise(EdgeIdx edge)                                const;

        bool contains (int node1, int node2)                                const;
        bool contains (AssignmentIdx assignment)                            const;
        
        bool contains (int node1, int node2, int node3, int node4)          const;
        bool contains (EdgeIdx edge)                                        const;

        void set_unary(int node1, int node2, double cost);
        void set_pairwise(int node1, int node2, int node3, int node4, double cost);

        const AssignmentContainer&  all_assignments()   const { return assignments; }
        const EdgeContainer&        all_edges()         const { return edges; }

        // rule of five
        CostMap(const CostMap& other)             = default;
        CostMap(CostMap&& other)                  = default;
        CostMap& operator=(const CostMap& other)  = default;
        CostMap& operator=(CostMap&& other)       = default;

    private:
        AssignmentContainer assignments;
        EdgeContainer edges;

        EdgeIdx sort_edge_indices(EdgeIdx idx) const;
};
}
#endif