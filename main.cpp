//============================================================================
// Name        : main.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
// 372cf4ab-9601-480c-92be-14272b10e1bb gurobi
//============================================================================

#include "optimizer/feasibility_characterization.h"
#define _HAS_STD_BYTE 0

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>

#include "libs/json.hpp"
#include "optimizer/optimizer.h"
#include "optimizer/optimizer_creator.h"
#include "preprocessing/instance_generator.h"
#include "preprocessing/reader.h"
#include "utils/const.h"
#include "utils/validation.h"
#include "utils/generate_config_files.h"
#include "gurobi_c++.h"

using json = nlohmann::json;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "[ERRO] Uso: \n\t (1) ./ajns 0 diretório algorithm \n\t"
            << "(2) ./ajns 1 arquivo_de_configuração\n"
            << "Use 0 se quiser gerar arquivos de configuração e "
            << "1 para resolver uma instância."
            << std::endl;
        return 1;
    }

    int action = std::atoi(argv[1]);
    if (action == 0) {
        if (argc < 8) {
            std::cerr << "[ERRO] Uso: ./ajns 0 dir algorithm "
                << "time_limit memory_limit num_threads verbosity"
                << std::endl;
            return 1;
        }
        std::string directory = argv[2];
        std::string algorithm = argv[3];
        long time_limit = std::atoi(argv[4]);
        long memory_limit = std::atoi(argv[5]);
        int num_threads = std::atoi(argv[6]);
        int verbosity = std::atoi(argv[7]);

        generate_json(directory, algorithm, time_limit, memory_limit, num_threads, verbosity);
        return 0;
    }

    std::ifstream input(argv[2]);
    if (!input.is_open()) {
        std::cerr << "[ERRO] Não foi possível abrir o arquivo " << argv[2]
            << std::endl;
        return 1;
    }

    json config;
    input >> config;
    AlgorithmIds ALGO_ID;

    try {
        validate_json(config);
    }
    catch (const std::exception& e) {
        std::cerr << "[ERRO] " << e.what();
        return 1;
    }
    std::cout << config["algo"]["type"] << std::endl;
    std::cout << ALGO_ID.str_to_enum[config["algo"]["type"]] << std::endl;

    AlgorithmType algorithm = ALGO_ID.str_to_enum[config["algo"]["type"]];
    std::cout << "antes " << config["algo"]["options"]["relaxed"] << std::endl;
    bool relaxed = config["algo"]["options"]["relaxed"];

    optimizer::SolverParameters* solver_parameters =
        new optimizer::SolverParameters(
            config["solver"]["options"]["time_limit"],
            config["solver"]["options"]["memory_limit"],
            config["solver"]["options"]["num_threads"],
            config["solver"]["options"]["verbosity"]);

    std::cout << "solver parameters " << std::endl;
    auto input_file = ajns::reader(config["infile_name"]);
    auto problem_data = input_file.read();
    auto generator = ajns::instance_generator();
    auto instance = generator.create_instance(problem_data);
    try
    {
        optimizer::Optimizer* optimizer = optimizer::OptimizerCreator::create(
            instance, algorithm, relaxed, *solver_parameters);

        if (optimizer == nullptr) {
            std::cerr << "[ERRO] Não foi possível instanciar o otimizador "
                << ALGO_ID.enum_to_str[algorithm] << std::endl;
            return 1;
        }
        optimizer->run();
        optimizer->set_info(instance.id, algorithm);
        optimizer->save_metrics("results/");

    }
    catch (GRBException& e)
    {
        std::cerr << "Gurobi Error: " << e.getMessage() << std::endl;
        return 1;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[ERRO] " << e.what();
        return 1;
    }

    return 0;
}