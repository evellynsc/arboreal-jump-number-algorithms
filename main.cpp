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
// IloCplex:: getNcols num of variables
// IloCplex:: getNrows num of constraints

using json = nlohmann::json;

int main(int argc, char* argv[]) {
    // ./ajns file algo_type relaxed [others]
    if (argc < 2) {
        std::cerr << "[ERRO] O JSON de configuração não foi especificado. Uso: "
                     "./ajns "
                     "file.json"
                  << std::endl;
        return 1;
    }

    std::ifstream input(argv[1]);
    if (!input.is_open()) {
        std::cerr << "[ERRO] Não foi possível abrir o arquivo " << argv[1]
                  << std::endl;
        return 1;
    }

    json config;
    input >> config;
    AlgorithmIds ALGO_ID;

    try {
        validate_json(config);
        std::cout << "[INFO] O JSON de configuração " << argv[1] << " é válido"
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
    } else if (algorithm == FEASIBILITY) {
        ((optimizer::FeasibilityCharacterization*)optimizer)
            ->set_num_jumps(config["algo"]["options"]["num_jumps"]);
    }
    optimizer->run();

    // auto start = std::chrono::high_resolution_clock::now();
    // auto prop = ajns::properties();

    // prop.num_arcs = my_instance.num_edges;
    // prop.num_nodes = my_instance.num_vertices;

    exit(1);
    // int opt = std::atoi(argv[2]);
    // switch (opt) {
    //     case 1: {
    //         prop.algo_t = algo_type::HH;
    //         auto extension = ajns::minimal_extension(my_instance);
    //         extension.run(prop);
    //         break;
    //     }
    //     case 2: {
    //         prop.algo_t = algo_type::BRANCH_AND_CUT;
    //         auto solver_config = solver::solver_params();
    //         auto exp_model =
    //             solver::ExponentialModel(my_instance,
    //             solver::CUTSET_EXP);
    //         solver::solver* ajnp_solver =
    //             new solver::BCSolver(solver_config, exp_model);
    //         try {
    //             std::cout << "entrando solver\n";
    //             ajnp_solver->solve(prop);
    //         } catch (...) {
    //             std::cout << "algo de errado não está certo\n";
    //         }
    //         break;
    //     }
    //     case 3: {
    //         prop.algo_t = algo_type::MFLOW;
    //         auto solver_config = solver::solver_params();
    //         auto exp_model = solver::MultiFlowModel(my_instance,
    //         relaxed); solver::solver* ajnp_solver =
    //             new solver::MFSolver(solver_config, exp_model);
    //         ajnp_solver->save(my_instance.id, "MPS");
    //         return 0;
    //         try {
    //             ajnp_solver->solve(prop);
    //         } catch (...) {
    //             std::cout << "algo de errado não está certo\n";
    //         }
    //         break;
    //     }
    //     case 4: {
    //         prop.algo_t = algo_type::DDL;
    //         auto solver_config = solver::solver_params();
    //         auto model = solver::DDLModel(my_instance, relaxed);
    //         solver::solver* ajnp_solver =
    //             new solver::DDLSolver(solver_config, model);
    //         try {
    //             ajnp_solver->solve(prop);
    //         } catch (...) {
    //             std::cout << "algo de errado não está certo\n";
    //         }
    //         break;
    //     }
    //     case 5: {
    //         prop.algo_t = algo_type::CHARACTERIZATION;
    //         auto solver_config = solver::solver_params();
    //         std::cout << "CHARACTERIZATION\n";
    //         auto ascending = true;
    //         if (std::atoi(argv[4]) == 0) ascending = false;
    //         // auto s = atoi(argv[3]);
    //         auto s = 1;
    //         auto run_next = true;
    //         if (ascending) {
    //             while (run_next and s != my_instance.num_vertices) {
    //                 auto model =
    //                 solver::CharacterizationBasedFormulation(
    //                     my_instance, s, relaxed);
    //                 solver::solver* ajnp_solver =
    //                     new
    //                     solver::CharacterizationBasedSolver(solver_config,
    //                                                             model);
    //                 try {
    //                     ajnp_solver->solve(prop);
    //                 } catch (...) {
    //                     std::cout << "algo de errado não está certo\n";
    //                 }
    //                 std::cout << ajnp_solver->get_status() << std::endl;
    //                 run_next = (ajnp_solver->get_status() == 1 ||
    //                             ajnp_solver->get_status() == 2)
    //                                ? false
    //                                : true;
    //                 s++;
    //             }
    //         } else {
    //             s = std::atoi(argv[5]);
    //             while (run_next and s != 0) {
    //                 auto model =
    //                 solver::CharacterizationBasedFormulation(
    //                     my_instance, s, relaxed);
    //                 solver::solver* ajnp_solver =
    //                     new
    //                     solver::CharacterizationBasedSolver(solver_config,
    //                                                             model);
    //                 try {
    //                     ajnp_solver->solve(prop);
    //                 } catch (...) {
    //                     std::cout << "algo de errado não está certo\n";
    //                 }
    //                 std::cout << ajnp_solver->get_status() << std::endl;
    //                 run_next = (ajnp_solver->get_status() == 1 ||
    //                             ajnp_solver->get_status() == 2)
    //                                ? false
    //                                : true;
    //                 s--;
    //             }
    //         }
    //         break;
    //     }
    //     case 6: {
    //         Kernel* search = new Kernel(my_instance, "flow", 4);
    //         search->run();
    //         break;
    //     }
    // }

    // auto end = std::chrono::high_resolution_clock::now();
    // double run_time =
    //     std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
    //         .count();  // @suppress("Invalid arguments") //
    //         @suppress("Symbol is
    //                    // not resolved") // @suppress("Method cannot be
    //                    // resolved")
    // run_time *= 1e-9;

    // prop.run_time = run_time;

    // auto outFile = std::ofstream();
    // auto name_file = "results" + std::to_string(prop.algo_t) + ".txt";
    // outFile.open(name_file, std::ofstream::out | std::ofstream::app);

    // outFile << my_instance.id << " " << prop.num_nodes << " " <<
    // prop.num_arcs
    //         << " " << prop.num_violators << " " << prop.num_jumps << " "
    //         << fixed << prop.run_time << setprecision(9)
    //         << std::endl;  // @suppress("Invalid overload")

    // std::cout << "time === " << fixed << run_time << setprecision(9)
    //           << " ===\n";
    return 0;
}
