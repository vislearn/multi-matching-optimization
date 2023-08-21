#ifndef LIBMGM_DATASTRUCTURES_HPP
#define LIBMGM_DATASTRUCTURES_HPP

#include <unordered_map>
#include <utility>
/*
* FIXME: Temporary Hash functions. Simply builds a string from input integers.
* Most definitely suboptimal.
*/
typedef std::pair<int,int> AssignmentIdx;
typedef std::pair<AssignmentIdx, AssignmentIdx> EdgeIdx;

struct AssignmentIdxHash {
    std::size_t operator()(AssignmentIdx const& input) const noexcept {
        std::string s = std::to_string(input.first) + ',' + std::to_string(input.second);
        std::size_t hash = std::hash<std::string>{}(s);
        
        return hash;
    }
};

struct EdgeIdxHash {
    std::size_t operator()(EdgeIdx const& input) const noexcept {
        std::string s = std::to_string(input.first.first)   + ',' +
                        std::to_string(input.first.second)  + ',' +
                        std::to_string(input.second.first)  + ',' +
                        std::to_string(input.second.second)  + ',';

        std::size_t hash = std::hash<std::string>{}(s);
        
        return hash;
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
        CostMap(int no_nodes_g1, int no_unaries, int no_pairwise);
        ~CostMap() {};
        
        const double& unary(int node1, int node2);
        const double& unary(AssignmentIdx assignment);
        
        const double& pairwise(int node1, int node2, int node3, int node4);
        const double& pairwise(EdgeIdx edge);

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
};

#endif