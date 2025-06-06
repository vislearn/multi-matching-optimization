#include <filesystem>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <regex>
#include <memory>
#include <stdexcept>

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
    std::shared_ptr<GmModel> parse_gm(std::ifstream& dd_file, int g1_id, int g2_id, double unary_constant=0.0);
}

std::shared_ptr<GmModel> parse_dd_file_gm(fs::path dd_file, double unary_constant) {
    if (unary_constant != 0.0) {
        spdlog::info("Loading model with custom unary constant: {}", unary_constant);
    }
    std::ifstream infile(dd_file);

    return details::parse_gm(infile, 0, 1, unary_constant);
}

std::shared_ptr<MgmModel> parse_dd_file(fs::path dd_file, double unary_constant) {
    if (unary_constant != 0.0) {
        spdlog::info("Loading model with custom unary constant: {}", unary_constant);
    }
    std::ifstream infile(dd_file);
    std::string line;
    std::stringstream lineStream;
    std::smatch re_match;

    if (infile.peek() == 'p') {
        spdlog::error("Given file begins with GM model definition. Missing 'gm <graph1_id> <graph2_id>'.");
        throw std::invalid_argument("Given file begins with GM model definition. Missing 'gm <graph1_id> <graph2_id>'.");
    }

    auto model = std::make_shared<MgmModel>();

    int max_graph_id = 0;
    while (std::getline(infile, line)) {
        if (std::regex_match(line, re_match, re_gm)) {
            int g1_id = std::stoi(re_match[1]);
            int g2_id = std::stoi(re_match[2]);
            if (g2_id > max_graph_id) {
                max_graph_id = g2_id;
                model->graphs.resize(max_graph_id+1);
            }
            spdlog::info("Graph {} and Graph {}", g1_id, g2_id);

            auto gmModel = details::parse_gm(infile, g1_id, g2_id, unary_constant);

            model->graphs[g1_id] = gmModel->graph1;
            model->graphs[g2_id] = gmModel->graph2;

            GmModelIdx idx(g1_id, g2_id);
            model->models[idx] = gmModel;
        }
    }
    model->no_graphs = max_graph_id + 1;

    spdlog::info("Finished parsing model.\n");
    return model;
}

void export_dd_file(fs::path dd_file, std::shared_ptr<MgmModel> model)
{
    spdlog::info("Exporting model as .dd file.\n");
    std::ofstream outfile(dd_file);
    outfile << std::setprecision(16);

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

json null_valued_labeling(const std::vector<int>& l) {
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

void save_to_disk(fs::path outPath, const MgmSolution& solution) {
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
    for (auto const& [key, gm_labeling] : solution.labeling()) {
        json json_labeling = null_valued_labeling(gm_labeling);

        std::string key_string = fmt::format("{}, {}", key.first, key.second);
        j["labeling"][key_string] = json_labeling; 
    }

    spdlog::debug("Saving solution to disk: {}", j.dump());

    if (fs::is_directory(outPath)) {
        outPath = outPath / "solution.json";
    }
    if (outPath.extension() != ".json") {
        outPath.replace_extension(".json");
    }
    fs::create_directories(outPath.parent_path());

    std::ofstream o(outPath);
    o << std::setw(4) << j << "\n";
    o.close();
}

void save_to_disk(fs::path outPath, const GmSolution &solution) {
   json j;

    // energy
    j["energy"] = solution.evaluate();
    
    // Number of nodes per graph:
    std::vector<int> graph_sizes = {solution.model->graph1.no_nodes, solution.model->graph2.no_nodes};
    j["graph orders"] = graph_sizes;

    // labeling
    j["labeling"] = null_valued_labeling(solution.labeling()); 

    spdlog::debug("Saving solution to disk: {}", j.dump());

    if (fs::is_directory(outPath)) {
        outPath = outPath / "solution.json";
    }
    if (outPath.extension() != ".json") {
        outPath.replace_extension(".json");
    }
    fs::create_directories(outPath.parent_path());
    
    std::ofstream o(outPath);
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

MgmSolution import_from_disk(fs::path labeling_path, std::shared_ptr<MgmModel> model) {
    MgmSolution s(model);
    Labeling l = s.create_empty_labeling();

    spdlog::info("Parsing json");
    std::ifstream ifs(labeling_path);
    json j = json::parse(ifs);

    for (auto& [key, arr] : j.at("labeling").items()) {
        GmModelIdx idx = from_json(key);

        if (l.find(idx) == l.end()) 
            throw std::invalid_argument("Provided model does not contain graph pair contained in labeling");

        auto& gm_labeling = l[idx];

        int i = 0;
        for (auto& val : arr) {
            if (!val.is_null())
                gm_labeling[i] = val.template get<int>();
            i++;
        }
    }
    s.set_solution(l);

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

std::shared_ptr<GmModel> parse_gm(std::ifstream& dd_file, int g1_id, int g2_id, double unary_constant) {
    std::string line = "";
    std::stringstream lineStream;

    // metadata of GM problem
    // p N0 N1 A E
    std::getline(dd_file, line);
    lineStream.clear();
    lineStream.str(line.substr(2));
    int no_left = 0;
    int no_right = 0;
    int no_a = 0;
    int no_e = 0;

    lineStream >> no_left >> no_right >> no_a >> no_e;
    
    Graph g1(g1_id, no_left);
    Graph g2(g2_id, no_right);

    auto gmModel = std::make_shared<GmModel>(g1, g2, no_a, no_e);

    int ass_id = 0;
    int id1 = 0;
    int id2 = 0;
    double c = 0.0;

    // Assignments
    // a A iA jA cA
    for (auto i = 0; i < no_a; i++) {
        std::getline(dd_file, line);
        lineStream.clear();
        lineStream.str(line.substr(2));
        lineStream >> ass_id >> id1 >> id2 >> c;

        assert ((size_t) ass_id == gmModel->assignment_list.size());
        gmModel->add_assignment(id1, id2, (c + unary_constant) );
    }

    // Edges
    // e a1 a′1 d1
    for (auto i = 0; i < no_e; i++) {
        std::getline(dd_file, line);
        lineStream.clear();
        lineStream.str(line.substr(2));
        lineStream >> id1 >> id2 >> c;
        
        gmModel->add_edge(id1, id2, c);
    }

    return gmModel;
}
}
}