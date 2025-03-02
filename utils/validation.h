#include <iostream>

#include "libs/json.hpp"

using json = nlohmann::json;

inline void validate_json(const json& data) {
    /**
     * @brief
    {
        "infile_name": "arquivo.m",
        "algo": {
            "type": "CHARACTERIZATION",
            "options": {
                "relaxed": false,
                "order": "ASC"
            }
        },
        "solver": {
            "name": "CPLEX",
            "options": {
                "time_limit": 3600,
                "memory_limit": 40000,
                "num_threads": 4
            }
        },
        "output": {
            "model_format": "lp",
            "outfile_name": "solucao.dot",
            "metrics": "metricas.json"
        }
    }
     *
     */
    if (!data.contains("infile_name")) {
        throw std::runtime_error(
            "Não foi possível encontrar a chave 'infile_name' no json de "
            "configuração.\n");
    }

    if (!data.contains("algo")) {
        throw std::runtime_error(
            "Não foi possível encontrar a chave 'algo' no json de "
            "configuração.\n");
    }
    if (!data.contains("solver")) {
        throw std::runtime_error(
            "Não foi possível encontrar a chave 'solver' no json de "
            "configuração.\n");
    }
    if (!data.contains("outfile_name")) {
        throw std::runtime_error(
            "Não foi possível encontrar a chave 'outfile_name' no json de "
            "configuração.\n");
    }
    // algorithm configuration
    if (!data["algo"].contains("type")) {
        throw std::runtime_error(
            "Não foi possível encontrar a chave 'algo.type' no json de "
            "configuração.\n");
    }

    if (!data["algo"].contains("options")) {
        throw std::runtime_error(
            "Não foi possível encontrar a chave 'algo.options' no json de "
            "configuração.\n");
    }

    if (!data["algo"]["options"].contains("relaxed")) {
        throw std::runtime_error(
            "Não foi possível encontrar a chave "
            "'algo.options.relaxed' "
            "no json de configuração.\n");
    }


    // if (data["algo"]["type"] == "FEASIBILITY") {
    //     if (!data["algo"]["options"].contains("num_jumps")) {
    //         throw std::runtime_error(
    //             "Não foi possível encontrar a chave "
    //             "'algo.options.num_parts' "
    //             "no json de configuração.\n");
    //     }
    //     if (!data["algo"]["options"].contains("order")) {
    //         throw std::runtime_error(
    //             "Não foi possível encontrar a chave "
    //             "'algo.options.order' "
    //             "no json de configuração.\n");
    //     }
    // }

    // solver configuration
    if (!data["solver"].contains("name")) {
        throw std::runtime_error(
            "Não foi possível encontrar a chave 'solver.name' no json de "
            "configuração.\n");
    }

    if (!data["solver"].contains("options")) {
        throw std::runtime_error(
            "Não foi possível encontrar a chave 'solver.options' no json "
            "de "
            "configuração.\n");
    }

    if (!data["solver"]["options"].contains("time_limit")) {
        throw std::runtime_error(
            "Não foi possível encontrar a chave "
            "'solver.options.time_limit' no "
            "json de configuração.\n");
    }

    if (!data["solver"]["options"].contains("memory_limit")) {
        throw std::runtime_error(
            "Não foi possível encontrar a chave "
            "'solver.options.memory_limit' "
            "no json de configuração.\n");
    }

    if (!data["solver"]["options"].contains("num_threads")) {
        throw std::runtime_error(
            "Não foi possível encontrar a chave "
            "'solver.options.num_threads' "
            "no json de configuração.\n");
    }
}