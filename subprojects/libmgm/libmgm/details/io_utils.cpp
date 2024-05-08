#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <regex>

#include <cassert>
#include <cstdio>

// Logging
#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/ranges.h> // print vector

// Fast hashmap
#include <ankerl/unordered_dense.h>

// json
#include <nlohmann/json.hpp>
namespace fs = std::filesystem;
using json = nlohmann::json;

#include "io_utils.hpp"
#include "solution.hpp"
#include "multigraph.hpp"
#include "costs.hpp"

namespace mgm::io {

const std::regex re_gm("^gm ([0-9]+) ([0-9]+)$");
const std::regex re_p("^p ([0-9]+) ([0-9]+) ([0-9]+) ([0-9]+)$");
const std::regex re_a("^a ([0-9]+) ([0-9]+) ([0-9]+) (.+)$");
const std::regex re_e("^e ([0-9]+) ([0-9]+) (.+)$");

// Forward declaration
namespace details {
    void write_model(std::ofstream& outfile, std::shared_ptr<GmModel> model);
}

MgmModel parse_dd_file(fs::path dd_file, double unary_constant) {
    if (unary_constant != 0.0) {
        spdlog::info("Loading model with custon unary constant: {}", unary_constant);
    }
    auto model = MgmModel();

    std::ifstream infile(dd_file);
    std::string line; 
    std::stringstream lineStream;
    std::smatch re_match;

    int max_graph_id = 0;
    while (std::getline(infile, line)) {
        if (std::regex_match(line, re_match, re_gm)) {
            int g1_id = std::stoi(re_match[1]);
            int g2_id = std::stoi(re_match[2]);
            if (g2_id > max_graph_id) {
                max_graph_id = g2_id;
                model.graphs.resize(max_graph_id+1);
            }
            spdlog::info("Graph {} and Graph {}", g1_id, g2_id);

            // metadata of GM problem
            std::getline(infile, line);
            lineStream.clear();
            lineStream.str(line.substr(2));
            int no_left = 0;
            int no_right = 0;
            int no_a = 0;
            int no_e = 0;

            lineStream >> no_left >> no_right >> no_a >> no_e;
            
            //FIXME: graphs with same id initialized multiple times over.
            Graph g1(g1_id, no_left);
            Graph g2(g2_id, no_right);
            model.graphs[g1_id] = g1;
            model.graphs[g2_id] = g2;

            GmModel gmModel(g1, g2, no_a, no_e);

            int ass_id = 0;
            int id1 = 0;
            int id2 = 0;
            double c = 0.0;

            // Assignments
            for (auto i = 0; i < no_a; i++) {
                std::getline(infile, line);
                lineStream.clear();
                lineStream.str(line.substr(2));
                lineStream >> ass_id >> id1 >> id2 >> c;

                assert ((size_t) ass_id == gmModel.assignment_list.size());
                gmModel.add_assignment(id1, id2, (c + unary_constant) );
            }

            // Edges
            for (auto i = 0; i < no_e; i++) {
                std::getline(infile, line);
                lineStream.clear();
                lineStream.str(line.substr(2));
                lineStream >> id1 >> id2 >> c;
                
                gmModel.add_edge(id1, id2, c);
            }

            GmModelIdx idx(g1_id, g2_id);
            model.models[idx] = std::make_shared<GmModel>(std::move(gmModel));
        }
    }
    model.no_graphs = max_graph_id + 1;

    spdlog::info("Finished parsing model.\n");
    return model;
}

MgmModel parse_dd_file_fscan(fs::path dd_file) {
    auto model = MgmModel();

    FILE* infile;
    infile = std::fopen(dd_file.c_str(), "r");
    fscanf(infile, " ");

    char BUF[1024];

    int max_graph_id = 0;
    char line_indicator[11];
    int g1_id = 0;
    int g2_id = 0;
    int ret = 0;
    while (std::fgets(BUF, sizeof(BUF), infile) != NULL) {
        ret = std::sscanf(BUF, "%10s %d %d\n", line_indicator, &g1_id, &g2_id);
        if (ret < 3) {
            continue;
        }
        if (strncmp(line_indicator, "gm", 2) != 0) {
            continue;
        }


        if (g2_id > max_graph_id) {
            max_graph_id = g2_id;
            model.graphs.resize(max_graph_id+1);
        }
        spdlog::info("Graph {} and Graph {}", g1_id, g2_id);

        int no_left = 0;
        int no_right = 0;
        int no_a = 0;
        int no_e = 0;

        ret = std::fscanf(infile, "%10s %d %d %d %d\n", line_indicator, &no_left, &no_right, &no_a, &no_e);

        assert(ret == 5);
        assert(strncmp(line_indicator, "p", 1) == 0);

        //FIXME: graphs with same id initialized multiple times over.
        Graph g1(g1_id, no_left);
        Graph g2(g2_id, no_right);
        model.graphs[g1_id] = g1;
        model.graphs[g2_id] = g2;

        GmModel gmModel(g1, g2, no_a, no_e);

        int ass_id = 0;
        int id1 = 0;
        int id2 = 0;
        double c = 0.0;

        // Assignments
        for (auto i = 0; i < no_a; i++) {
            ret = std::fscanf(infile, "%10s %d %d %d %lf\n", line_indicator, &ass_id, &id1, &id2, &c);
            assert(ret == 5);
            assert(strncmp(line_indicator, "a", 1) == 0);
            
            assert ((size_t) ass_id == gmModel.assignment_list.size());
            gmModel.add_assignment(id1, id2, c);
        }

        // Edges
        for (auto i = 0; i < no_e; i++) {
            ret = std::fscanf(infile, "%10s %d %d %lf\n", line_indicator, &id1, &id2, &c);
            assert(ret == 4);
            assert(strncmp(line_indicator, "e", 1) == 0);
            gmModel.add_edge(id1, id2, c);
        }

        GmModelIdx idx(g1_id, g2_id);
        model.models[idx] = std::make_shared<GmModel>(std::move(gmModel));
    }
    model.no_graphs = max_graph_id + 1;

    fclose(infile);
    return model;
}

void export_dd_file(std::filesystem::path dd_file, std::shared_ptr<MgmModel> model)
{
    spdlog::info("Exporting model as .dd file.\n");
    std::ofstream outfile(dd_file);

    // Edge case, just one model present.
    if (model->models.size() == 1){
        auto gm_model = model->models.begin()->second;
        details::write_model(outfile, gm_model);

        outfile.close();
        return;
    }
    
    // Sort keys of models for exporting
    std::vector<GmModelIdx> keys;
    for (const auto& pair : model->models) {
        keys.push_back(pair.first);
    }
    std::sort(keys.begin(), keys.end());

    for (const auto& gm_model_idx : keys) {
        auto m = model->models[gm_model_idx];
        spdlog::info("Exporting pair ({} {})", m->graph1.id, m->graph2.id);
        outfile << "gm " << m->graph1.id << " " << m->graph2.id << "\n";
        details::write_model(outfile, m);
    }

    outfile.close();
    spdlog::info("Finished exporting.\n");
}

json null_valued_labeling(std::vector<int> l) {
    json new_l;
    for (const auto& value : l) {
        if (value == -1) {
            new_l.push_back(nullptr);
        } else {
            new_l.push_back(value);
        }
    }
    return new_l;
}

void safe_to_disk(const MgmSolution& solution, fs::path outPath, std::string filename) {
   json j;

    // energy
    j["energy"] = solution.evaluate();
    
    // Number of nodes per graph:
    std::vector<int> graph_sizes;
    for (const auto & g : solution.model->graphs) {
        graph_sizes.push_back(g.no_nodes);
    }
    j["graph orders"] = graph_sizes;

    // labeling
    for (auto const& [key, s] : solution.gmSolutions) {
        json json_labeling = null_valued_labeling(s.labeling);

        std::string key_string = fmt::format("{}, {}", s.model->graph1.id, s.model->graph2.id);
        j["labeling"][key_string] = json_labeling; 
    }


    spdlog::debug("Saving solution to disk: {}", j.dump());
    std::ofstream o(outPath / (filename + ".json"));
    o << std::setw(4) << j << "\n";
    o.close();
}

GmModelIdx from_json(const std::string input) {
    std::string g1, g2;
    std::istringstream ss(input);
    
    std::getline(ss, g1, ',');
    std::getline(ss, g2, ',');

    return GmModelIdx(std::stoi(g1), std::stoi(g2));
}

MgmSolution import_from_disk(std::shared_ptr<MgmModel> model, fs::path labeling_path) {
    MgmSolution s(model);

    spdlog::info("Parsing json");
    std::ifstream ifs(labeling_path);
    json j = json::parse(ifs);

    for (auto& [key, arr] : j.at("labeling").items()) {
        GmModelIdx idx = from_json(key);
        auto& gm_labeling = s.gmSolutions[idx].labeling;

        int i = 0;
        for (auto& val : arr) {
            if (!val.is_null())
                gm_labeling[i] = val.template get<int>();
            i++;
        }
    }
    if (!j.at("energy").is_null()){
        double j_energy = j.at("energy").template get<double>();
        spdlog::debug("Energy according to json: {}", j_energy);
        }
    else {
        spdlog::debug("Energy according to json: null");
    }

    spdlog::debug("Energy of parsed model: {}", s.evaluate());
    return s;
}

namespace details {
    void write_model(std::ofstream& outfile, std::shared_ptr<GmModel> model) {
        
        outfile << "p "
                << model->graph1.no_nodes << " "
                << model->graph2.no_nodes << " "
                << model->no_assignments() << " "
                << model->no_edges() << "\n";

        // Cache the assignment id for writing.
        // Needs to be looked up for edges
        ankerl::unordered_dense::map<AssignmentIdx, int, AssignmentIdxHash> assignment_ids;

        int a_id = 0;
        for (const auto& a_idx : model->assignment_list) {
            auto & cost = model->costs->unary(a_idx);
            outfile << "a "
                    << a_id << " " 
                    << a_idx.first << " " 
                    << a_idx.second << " " 
                    << cost << "\n";

            assignment_ids[a_idx] = a_id;
            a_id++;
        }
        
        for (const auto& [edge_idx, cost] : model->costs->all_edges()) {
            outfile << "e "
                    << assignment_ids[edge_idx.first]   << " " 
                    << assignment_ids[edge_idx.second]  << " " 
                    << cost << "\n";
        }
    }
}
}