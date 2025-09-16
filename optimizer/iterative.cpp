#include "iterative.h"
#include "utils/const.h"
#include <cmath>
#include <iostream>

namespace optimizer {

Iterative::Iterative(ajns::Instance& _instance, bool _relaxed, SolverParameters& _parameters)
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

void Iterative::add_variables() {
    auto n = this->instance.num_vertices;
    auto r = instance.input_graph[instance.root].id;

    x.resize(n * n);
    y.resize(n * n * (t_max + 1));
    u.resize(n * n * n * (t_max + 1));

    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            x[j + i * n] = gurobi_model->addVar(0.0, 1.0, 0.0, this->relaxed ? GRB_CONTINUOUS : GRB_BINARY);
            std::string name = "x_" + std::to_string(i) + "_" + std::to_string(j);
            x[j + i * n].set(GRB_StringAttr_VarName, name);
        }
    }

    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            for (size_t t = 0; t < (t_max + 1); t++) {
                y[j + i * n + t * n * n] = gurobi_model->addVar(0.0, 1.0, 0.0, this->relaxed ? GRB_CONTINUOUS : GRB_BINARY);
                std::string name = "y_" + std::to_string(i) + "_" + std::to_string(j) + "_" + std::to_string(t);
                y[j + i * n + t * n * n].set(GRB_StringAttr_VarName, name);
            }
        }
    }

    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            for (size_t k = 0; k < n; k++) {
                for (size_t t = 0; t < (t_max + 1); t++) {
                    u[j + i * n + k * n * n + t * n * n * n] = gurobi_model->addVar(0.0, 1.0, 0.0, this->relaxed ? GRB_CONTINUOUS : GRB_BINARY);
                    std::string name = "u_" + std::to_string(i) + "_" + std::to_string(j) + "_" + std::to_string(k) + "_" + std::to_string(t);
                    u[j + i * n + k * n * n + t * n * n * n].set(GRB_StringAttr_VarName, name);
                }
            }
        }
    }

    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            auto edge_in_input_graph = instance.exist_edge_in_input_graph(i, j);
            if (!edge_in_input_graph || j == r) {
                gurobi_model->addConstr(x[n * i + j] == 0);
            }
        }
    }
}

void Iterative::add_objective_function() {
    auto n = this->instance.num_vertices;
    GRBLinExpr minimize_jumps = 0;
    for (const auto& e : boost::make_iterator_range(boost::edges(instance.input_graph))) {
        if (instance.input_graph[e].type == my_graph::ARTIFICIAL) {
            auto from = boost::source(e, instance.input_graph);
            auto to = boost::target(e, instance.input_graph);
            minimize_jumps += x[n * to + from];
        }
    }
    gurobi_model->setObjective(minimize_jumps, GRB_MINIMIZE);
}

void Iterative::add_constraints() {
    auto n = this->instance.num_vertices;
    auto r = instance.input_graph[instance.root].id;

    for (size_t j = 0; j < n; j++) {
        if (j != r) {
            GRBLinExpr sum_i = 0;
            for (size_t i = 0; i < n; i++) {
                sum_i += x[n * i + j];
            }
            gurobi_model->addConstr(sum_i == 1);
        }
    }

    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            gurobi_model->addConstr(x[j + n * i] == y[j + n * i]);
        }
    }

    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            for (size_t t = 0; t < t_max + 1; t++) {
                gurobi_model->addConstr(y[j + n * i + t * n * n] + y[i + n * j + t * n * n] <= 1);
            }
        }
    }

    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            for (size_t t = 1; t < t_max + 1; t++) {
                if (i != j) {
                    GRBLinExpr sum_k = 0;
                    for (size_t k = 0; k < n; k++) {
                        sum_k += u[k + n * i + n * n * j + (t - 1) * n * n * n];
                    }
                    gurobi_model->addConstr(y[j + n * i + t * n * n] <= y[j + n * i + (t - 1) * n * n] + sum_k);
                }
            }
        }
    }

    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            if (i != j) {
                for (size_t t = 1; t < t_max + 1; t++) {
                    gurobi_model->addConstr(y[j + n * i + t * n * n] >= y[j + n * i + (t - 1) * n * n]);
                }
            }
        }
    }

    for (size_t i = 0; i < n; i++) {
        for (size_t k = 0; k < n; k++) {
            for (size_t j = 0; j < n; j++) {
                for (size_t t = 1; t < t_max + 1; t++) {
                    gurobi_model->addConstr(y[j + n * i + t * n * n] >= u[k + n * i + n * n * j + (t - 1) * n * n * n]);
                }
            }
        }
    }

    for (size_t i = 0; i < n; i++) {
        for (size_t k = 0; k < n; k++) {
            for (size_t j = 0; j < n; j++) {
                for (size_t t = 0; t < t_max + 1; t++) {
                    gurobi_model->addConstr(u[i + n * k + n * n * j + t * n * n * n] <= y[i + n * k + t * n * n]);
                    gurobi_model->addConstr(u[i + n * k + n * n * j + t * n * n * n] <= y[j + n * i + t * n * n]);
                    gurobi_model->addConstr(u[i + n * k + n * n * j + t * n * n * n] >= y[i + n * k + t * n * n] + y[j + n * i + t * n * n] - 1);
                }
            }
        }
    }

    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            auto edge_in_order = instance.exist_edge_in_order_graph(i, j);
            if (edge_in_order && i != j) {
                gurobi_model->addConstr(y[j + n * i + t_max * n * n] == 1);
            }
        }
    }
}

void Iterative::extract_solution() {
    auto n = this->instance.num_vertices;
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            if (i != j) {
                if (x[n * i + j].get(GRB_DoubleAttr_X) > 1e-6) {
                    std::cout << i << ", " << j << std::endl;
                }
            }
        }
    }
}

}  // namespace optimizer