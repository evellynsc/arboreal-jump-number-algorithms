#include "iterative.h"

#include "ilconcert/iloexpression.h"
#include "utils/const.h"
namespace optimizer {

Iterative::Iterative(ajns::Instance& _instance, bool _relaxed,
                     SolverParameters& _parameters)
    : Optimizer(_instance, ITERATIVE, _relaxed, _parameters) {
    this->t_max = std::floor(std::log2(instance.num_vertices));
}

Iterative::Iterative(ajns::Instance& _instance, bool _relaxed)
    : Optimizer(_instance, ITERATIVE, _relaxed) {
    this->t_max = std::floor(std::log2(instance.num_vertices));
}

Iterative::Iterative(ajns::Instance& _instance)
    : Optimizer(_instance, ITERATIVE, false) {
    this->t_max = std::floor(std::log2(instance.num_vertices));
}

void Iterative::add_objective_function() {
    auto n = this->instance.num_vertices;
    IloExpr minimize_jumps(env);
    for (const auto& e :
         boost::make_iterator_range(boost::edges(instance.input_graph))) {
        if (instance.input_graph[e].type == my_graph::ARTIFICIAL) {
            auto from = boost::source(e, instance.input_graph);
            auto to = boost::target(e, instance.input_graph);
            minimize_jumps += x[n * to + from];
        }
    }
    /*for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            minimize_jumps += (int)i * x[n * i + j];
        }
    }*/
    this->cplex_model.add(IloMinimize(env, minimize_jumps));
    minimize_jumps.end();
}

void Iterative::add_constraints() {
    auto n = this->instance.num_vertices;
    auto r = instance.input_graph[instance.root].id;

    for (auto j = 0u; j < n; j++) {
        if (j != r) {
            IloExpr sum_i(env);
            for (auto i = 0u; i < n; i++) {
                sum_i += x[n * i + j];
            }
            this->cplex_model.add(sum_i == 1);
        }
    }

    for (auto i = 0u; i < n; i++) {
        for (auto j = 0u; j < n; j++) {
            this->cplex_model.add(x[j + n * i] == y[j + n * i]);
        }
    }

    for (auto i = 0u; i < n; i++) {
        for (auto j = 0u; j < n; j++) {
            // if (i != j) {
            for (auto t = 0u; t < t_max + 1; t++) {
                this->cplex_model.add(
                    y[j + n * i + t * n * n] + y[i + n * j + t * n * n] <= 1);
            }
            //}
        }
    }

    for (auto i = 0u; i < n; i++) {
        for (auto j = 0u; j < n; j++) {
            for (auto t = 1u; t < t_max + 1; t++) {
                if (i != j) {
                    IloExpr sum_k(env);
                    std::string sk = "";
                    for (auto k = 0u; k < n; k++) {
                        // if (i != k and j != k and i != j) {
                        sk += " + ";
                        sk += std::string(
                            u[k + n * i + n * n * j + (t - 1) * n * n * n]
                                .getName());
                        sum_k += u[k + n * i + n * n * j + (t - 1) * n * n * n];
                        //}
                    }

                    this->cplex_model.add(y[j + n * i + t * n * n] <=
                                          y[j + n * i + (t - 1) * n * n] +
                                              sum_k);
                }
            }
        }
    }

    for (auto i = 0u; i < n; i++) {
        for (auto j = 0u; j < n; j++) {
            if (i != j) {
                for (auto t = 1u; t < t_max + 1; t++) {
                    this->cplex_model.add(y[j + n * i + t * n * n] >=
                                          y[j + n * i + (t - 1) * n * n]);
                }
            }
        }
    }

    for (auto i = 0u; i < n; i++) {
        for (auto k = 0u; k < n; k++) {
            for (auto j = 0u; j < n; j++) {
                // if (i != k and k != j and i != j) {
                for (auto t = 1u; t < t_max + 1; t++) {
                    this->cplex_model.add(
                        y[j + n * i + t * n * n] >=
                        u[k + n * i + n * n * j + (t - 1) * n * n * n]);
                }
                //}
            }
        }
    }

    for (auto i = 0u; i < n; i++) {
        for (auto k = 0u; k < n; k++) {
            for (auto j = 0u; j < n; j++) {
                // if (i != k and k != j and i != j) {
                for (auto t = 0u; t < t_max + 1; t++) {
                    this->cplex_model.add(
                        u[i + n * k + n * n * j + t * n * n * n] <=
                        y[i + n * k + t * n * n]);
                    this->cplex_model.add(
                        u[i + n * k + n * n * j + t * n * n * n] <=
                        y[j + n * i + t * n * n]);
                    this->cplex_model.add(
                        u[i + n * k + n * n * j + t * n * n * n] >=
                        y[i + n * k + t * n * n] + y[j + n * i + t * n * n] -
                            1);
                }
                //}
            }
        }
    }
    for (auto i = 0u; i < n; i++) {
        for (auto j = 0u; j < n; j++) {
            auto edge_in_order = instance.exist_edge_in_order_graph(i, j);
            if (edge_in_order and i != j) {
                this->cplex_model.add(y[j + n * i + t_max * n * n] == 1);
            }
        }
    }
}

void Iterative::add_variables() {
    std::cout << "[INFO] t_max " << this->t_max << std::endl;
    auto r = instance.input_graph[instance.root].id;
    auto n = this->instance.num_vertices;
    std::cout << n << " " << t_max << std::endl;

    auto var_type = ILOBOOL;

    if (this->relaxed) var_type = ILOFLOAT;

    x = IloNumVarArray(env, n * n, 0.0, 1.0, var_type);
    y = IloNumVarArray(env, n * n * (t_max + 1u), 0.0, 1.0, var_type);
    u = IloNumVarArray(env, n * n * n * (t_max + 1u), 0.0, 1.0, var_type);

    for (auto i = 0; i < n; i++) {
        for (auto j = 0; j < n; j++) {
            std::string name =
                "x_" + std::to_string(i) + "_" + std::to_string(j);
            x[j + i * n].setName(name.c_str());
        }
    }

    for (auto i = 0; i < n; i++) {
        for (auto j = 0; j < n; j++) {
            for (auto t = 0; t < (t_max + 1); t++) {
                std::string name = "y_" + std::to_string(i) + "_" +
                                   std::to_string(j) + "_" + std::to_string(t);
                y[j + i * n + t * n * n].setName(name.c_str());
            }
        }
    }

    for (auto i = 0; i < n; i++) {
        for (auto j = 0; j < n; j++) {
            for (auto k = 0; k < n; k++) {
                for (auto t = 0; t < (t_max + 1); t++) {
                    std::string name =
                        "u_" + std::to_string(i) + "_" + std::to_string(j) +
                        "_" + std::to_string(k) + "_" + std::to_string(t);
                    u[j + i * n + k * n * n + t * n * n * n].setName(
                        name.c_str());
                }
            }
        }
    }

    for (auto i = 0; i < n; i++) {
        for (auto j = 0; j < n; j++) {
            auto edge_in_input_graph = instance.exist_edge_in_input_graph(i, j);
            if (!edge_in_input_graph or j == r) {
                this->cplex_model.add(x[n * i + j] == 0);
            }
        }
    }
}

void Iterative::extract_solution() {
    std::cout << "[INFO] Construindo solução" << std::endl;
    auto n = this->instance.num_vertices;
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            if (i != j) {
                if (cplex_solver.getValue(this->x[n * i + j]) > 1e-6) {
                    std::cout << i << ", " << j << std::endl;
                }
            }
        }
    }
}

}  // namespace optimizer