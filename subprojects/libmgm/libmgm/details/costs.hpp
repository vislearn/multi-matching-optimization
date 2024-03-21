#ifndef LIBMGM_COSTS_HPP
#define LIBMGM_COSTS_HPP

#include <unordered_map>
#include <utility>
#include <ankerl/unordered_dense.h>

namespace mgm {
/*
* FIXME: Using boost hash combine improved performance drastically. 
*   However, this could probably be improved with a more specialize Hashtable for large number of entries (number of edges ~500.000).
*/
typedef std::pair<int,int> AssignmentIdx;
typedef std::pair<AssignmentIdx, AssignmentIdx> EdgeIdx;


void boost_hash_combine(size_t& seed, const int& v);

struct AssignmentIdxHash {
    using is_avalanching = void;
    std::uint64_t operator()(AssignmentIdx const& input) const noexcept {
        std::uint64_t hash = input.first;
        hash = (hash << 16) | input.second;
        return ankerl::unordered_dense::detail::wyhash::hash(hash);
    }
};

struct EdgeIdxHash {
    using is_avalanching = void;
    std::size_t operator()(EdgeIdx const& input) const noexcept {
        std::uint64_t hash = input.first.first;
        hash = (hash << 16) | input.first.second;
        hash = (hash << 16) | input.second.first;
        hash = (hash << 16) | input.second.second;
        return ankerl::unordered_dense::detail::wyhash::hash(hash);
    }
};

typedef ankerl::unordered_dense::map<AssignmentIdx, double, AssignmentIdxHash> AssignmentContainer;
typedef ankerl::unordered_dense::map<EdgeIdx, double, EdgeIdxHash> EdgeContainer;

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