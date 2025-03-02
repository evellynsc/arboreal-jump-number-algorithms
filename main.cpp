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

using json = nlohmann::json;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "[ERRO] Uso: \n\t (1) ./ajns 0 diretório algorithm \n\t"
                  << "(2) ./ajns 1 arquivo_de_configuração"
                  << "Use 0 se quiser gerar arquivos de configuração e " 
                  << "1 para resolver uma instância."
                  << std::endl;
        return 1;
    }

    int action = std::atoi(argv[1]);
    if (action == 0) {
        std::string directory = argv[2];
        std::string algorithm = argv[3];
        std::cout << "[INFO] Gerando arquivos de configuração"
                  << std::endl;
        generate_json(directory, algorithm);
        std::cout << "[INFO] Arquivos de configuração salvos com sucesso"
                  << std::endl;
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
        std::cout << "[INFO] O JSON de configuração " << argv[2] << " é válido"
                  << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[ERRO] " << e.what();
        return 1;
    }

    AlgorithmType algorithm = ALGO_ID.str_to_enum[config["algo"]["type"]];
    bool relaxed = config["algo"]["options"]["relaxed"];

    optimizer::SolverParameters* solver_parameters =
        new optimizer::SolverParameters(
            config["solver"]["options"]["time_limit"],
            config["solver"]["options"]["memory_limit"],
            config["solver"]["options"]["num_threads"]);

    std::cout << "[INFO] Lendo arquivo de entrada " << config["infile_name"]
              << std::endl;
    auto input_file = ajns::reader(config["infile_name"]);
    auto problem_data = input_file.read();
    auto generator = ajns::instance_generator();
    auto instance = generator.create_instance(problem_data);

    optimizer::Optimizer* optimizer = optimizer::OptimizerCreator::create(
        instance, algorithm, relaxed, *solver_parameters);

    if (optimizer == nullptr) {
        std::cerr << "[ERRO] Não foi possível instanciar o otimizador "
                  << ALGO_ID.enum_to_str[algorithm] << std::endl;
        return 1;
    } 
    optimizer->run();
    return 0;
}
