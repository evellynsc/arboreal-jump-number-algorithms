#ifndef UTILS_GENERATE_CONFIG_FILES_CPP_
#define UTILS_GENERATE_CONFIG_FILES_CPP_


#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include "libs/json.hpp"
#include "utils/preprocessing.h"

namespace fs = std::filesystem;
using json = nlohmann::json;

void generate_json(const std::string& directory, const std::string& algorithm) {
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().string();
            json j;
            j["infile_name"] = filename;
            j["algo"] = {
                {"type", algorithm},
                {"options", {{"relaxed", false}}}
            };
            j["solver"] = {
                {"name", "CPLEX"},
                {"options", {
                    {"time_limit", 18000},
                    {"memory_limit", 20000},
                    {"num_threads", 4}
                }}
            };
            j["outfile_name"] = "";

            auto instance_name = ajns::get_filename_from_path(filename);

            std::string output_filename = "config/" + instance_name + ".json";
            std::cout << "[INFO] Salvando arquivo " << output_filename
                  << std::endl;
            std::ofstream output_file(output_filename);
            output_file << j.dump(4);
            output_file.close();
        }
    }
}


#endif