#ifndef OUTPUT_METRICS_H_
#define OUTPUT_METRICS_H_

#include <map>

struct Metrics {
    double solve_time;           // tempo de execução
    double num_jumps;               // número de jumps
    std::string instance_name;   // nome da instância
    int num_explored_nodes;      // quantidade de nós explorados
    std::map<std::string, int> num_cuts; // hash map: chave é string, valor é inteiro
    double primal_bound;         // limite primal
    double dual_bound;           // limite dual
    std::string algorithm_name; // nome do algoritmo
    std::string status;         // status da solução
    double cuts_added_by_solver;


    Metrics()
        : solve_time(0.0), num_jumps(0), instance_name(""),
          num_explored_nodes(0), num_cuts(std::map<std::string, int>()),
          primal_bound(0.0), dual_bound(0.0), algorithm_name("") {}

    std::string to_string() {
      return instance_name + ";" +
            algorithm_name + ";" +
            std::to_string(solve_time) + ";" +
            std::to_string(num_jumps) + ";" +
            std::to_string(num_explored_nodes) + ";" +
            std::to_string(primal_bound) + ";" +
            std::to_string(dual_bound) + ";" +
            std::to_string(cuts_added_by_solver) + ";" +
            status;
    }
};

#endif /* OUTPUT_METRICS_H_ */
