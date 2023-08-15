#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <regex>

// Logging
#include "spdlog/spdlog.h"

#include "multigraph.hpp"

using namespace std;

const regex re_gm("^gm ([0-9]+) ([0-9]+)$");
const regex re_p("^p ([0-9]+) ([0-9]+) ([0-9]+) ([0-9]+)$");
const regex re_a("^a ([0-9]+) ([0-9]+) ([0-9]+) (.+)$");
const regex re_e("^e ([0-9]+) ([0-9]+) (.+)$");

MgmModel parse_dd_file(std::filesystem::path dd_file) {
    auto model = MgmModel();

    ifstream infile(dd_file);
    string line; 
    std::stringstream lineStream;
    smatch re_match;

    cout << "start" << endl;
    while (getline(infile, line)) {
        if (regex_match(line, re_match, re_gm)) {
            int g1_id = stoi(re_match[1]);
            int g2_id = stoi(re_match[2]);

            spdlog::info("Graph {} and Graph {}", g1_id, g2_id);

            // metadata of GM problem
            getline(infile, line);
            lineStream.clear();
            lineStream.str(line.substr(2));
            int no_left = 0;
            int no_right = 0;
            int no_a = 0;
            int no_e = 0;

            lineStream >> no_left >> no_right >> no_a >> no_e;
            
            Graph g1(g1_id, no_left);
            Graph g2(g2_id, no_right);
            GmModel gmModel(g1, g2, no_a, no_e);

            int ass_id = 0;
            int id1 = 0;
            int id2 = 0;
            double c = 0.0;

            // Assignments
            for (auto i = 0; i < no_a; i++) {
                getline(infile, line);
                lineStream.clear();
                lineStream.str(line.substr(2));
                lineStream >> ass_id >> id1 >> id2 >> c;
                
                AssignmentIdx a(id1, id2);
                
                gmModel.assignment_list.push_back(a);
                gmModel.costs->set_unary(id1, id2, c);
                gmModel.assignments_left[id1].push_back(id2);
                gmModel.assignments_right[id2].push_back(id1);
            }

            // Edges
            for (auto i = 0; i < no_e; i++) {
                getline(infile, line);
                lineStream.clear();
                lineStream.str(line.substr(2));
                lineStream >> id1 >> id2 >> c;
                
                AssignmentIdx a1 = gmModel.assignment_list[id1];
                AssignmentIdx a2 = gmModel.assignment_list[id2];
                EdgeIdx e(a1, a2);
                gmModel.costs->set_pairwise(a1.first, a1.second, a2.first, a2.second, c);
                gmModel.costs->set_pairwise(a2.first, a2.second, a1.first, a1.second, c); //FIXME: RAM overhead. Avoids sorting later though.
                gmModel.edge_list.push_back(e);
            }

            GmModelIdx idx(g1_id, g2_id);
            model.models[idx] = std::make_shared<GmModel>(std::move(gmModel));
        }
    }

    return model;
}