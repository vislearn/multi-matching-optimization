#ifndef LIBMGM_SOLUTION_HPP
#define LIBMGM_SOLUTION_HPP

#include <unordered_map>
#include <vector>
#include <memory>
#include <utility>

#include "cliques.hpp"
#include "multigraph.hpp"

namespace mgm {

using Labeling = std::unordered_map<GmModelIdx, std::vector<int>, GmModelIdxHash>;

class GmSolution {
    public:
        GmSolution() = default;
        GmSolution(std::shared_ptr<GmModel> model);
        GmSolution(std::shared_ptr<GmModel> model, std::vector<int> labeling);

        static double evaluate(const GmModel& model, const std::vector<int>& labeling);
        double evaluate() const;

        int& operator[](int idx); // access labeling
        const int& operator[](int idx) const; // access labeling

        const std::vector<int>& labeling() const; // For interface consistency with MgmSolution
        std::vector<int>& labeling(); // For interface consistency with MgmSolution
        
        std::shared_ptr<GmModel> model;

    private:
        bool is_active(AssignmentIdx assignment) const;
        static bool is_active(const AssignmentIdx& assignment, const std::vector<int>& labeling);

        std::vector<int> labeling_;
};

class MgmSolution {
    public:
        MgmSolution(std::shared_ptr<MgmModel> model);

        //std::unordered_map<GmModelIdx, GmSolution, GmModelIdxHash> gmSolutions;
        std::shared_ptr<MgmModel> model;

        const std::vector<int>& operator[](GmModelIdx idx) const;

        double evaluate() const;
        double evaluate(int graph_id) const; // limit cost evaluation to models with graph `graph_id`.
        //bool is_cycle_consistent() const;

        // A solution can be represented in multiple valid ways
        // This class uses lazy evaluation and converts between different forms on demand.
        const Labeling&        labeling()          const;
        const CliqueManager&   clique_manager()    const;
        const CliqueTable&     clique_table()      const;

        void set_solution(const Labeling& labeling);
        void set_solution(const CliqueManager& clique_manager);
        void set_solution(const CliqueTable& clique_table);

        void set_solution(Labeling&& labeling);
        void set_solution(CliqueManager&& clique_manager);
        void set_solution(CliqueTable&& clique_table);

        void set_solution(const GmModelIdx& idx, std::vector<int> labeling);
        void set_solution(const GmSolution& sub_solution);

        Labeling create_empty_labeling() const;
    private:

        mutable bool labeling_valid         = false;
        mutable bool clique_manager_valid   = false;
        mutable bool clique_table_valid     = false;

        mutable Labeling        labeling_;
        mutable CliqueManager   cm;
        mutable CliqueTable     ct;


};

}
#endif