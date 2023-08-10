#ifndef LIBMGM_DATASTRUCTURES_HPP
#define LIBMGM_DATASTRUCTURES_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

// Logging
#include "spdlog/spdlog.h"

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

class ICostStructure {
    public:
        virtual ~ICostStructure() = 0;
        
        virtual double& unary(int node1, int node2) = 0;
        virtual double& pairwise(int node1, int node2, int node3, int node4) = 0;

    protected:
        // rule of five
        ICostStructure()                                        = default;
        ICostStructure(const ICostStructure& other)             = default;
        ICostStructure(ICostStructure&& other)                  = default;
        ICostStructure& operator=(const ICostStructure& other)  = default;
        ICostStructure& operator=(ICostStructure&& other)       = default;
};

class CostMap : public ICostStructure {
    public:
        CostMap(int no_unaries, int no_pairwise);
        ~CostMap() {};
        
        double& unary(int node1, int node2) override;
        double& pairwise(int node1, int node2, int node3, int node4) override;
        
        // rule of five
        CostMap(const CostMap& other)             = default;
        CostMap(CostMap&& other)                  = default;
        CostMap& operator=(const CostMap& other)  = default;
        CostMap& operator=(CostMap&& other)       = default;
    private:
        std::unordered_map<AssignmentIdx, double, AssignmentIdxHash> assignments;
        std::unordered_map<EdgeIdx, double, EdgeIdxHash> edges;
};

#endif