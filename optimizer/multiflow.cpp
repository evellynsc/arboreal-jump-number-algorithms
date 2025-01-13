/*
 * multiflow.cpp
 *
 *  Created on: Feb 23, 2022
 *      Author: evellyn
 */

#include "multiflow.h"

#include <iostream>
#include <numeric>

#include "optimizer/optimizer.h"
#include "utils/const.h"

namespace optimizer {

MultiFlow::MultiFlow(ajns::Instance& _instance)
    : Optimizer(_instance, MULTIFLOW, false) {}

MultiFlow::MultiFlow(ajns::Instance& _instance, bool _relaxed)
    : Optimizer(_instance, MULTIFLOW, _relaxed) {}

MultiFlow::MultiFlow(ajns::Instance& _instance, bool _relaxed,
                     SolverParameters& _parameters)
    : Optimizer(_instance, MULTIFLOW, _relaxed, _parameters) {}

IloNumVarArray MultiFlow::get_y_variables() { return this->var_x; }

void MultiFlow::reset_upper_bounds(std::vector<bool> zero_variables) {
    for (auto e :
         boost::make_iterator_range(boost::edges(this->instance.input_graph))) {
        int i = this->instance.input_graph[e].source_id;
        int j = this->instance.input_graph[e].target_id;
        int n = this->instance.num_vertices;
        int idx = n * i + j;
        if (zero_variables[idx]) {
            this->var_x[idx].setUB(0);
        }
    }
}

void MultiFlow::add_variables() {
    // auto r = this->instance.input_graph[this->instance.root].id;
    auto n = this->instance.num_vertices;

    std::cout << "[INFO] Adicionando variáveis f" << std::endl;
    this->var_f = IloNumVarArray(this->env, n * n * n);
    for (auto i = 0u; i < n; i++) {
        for (auto j = 0u; j < n; j++) {
            for (auto k = 0u; k < n; k++) {
                this->var_f[i + n * j + k * n * n] =
                    IloNumVar(this->env, 0.0, 1.0, ILOFLOAT);
                std::string var_name = "flow_" + std::to_string(i) + "_" +
                                       std::to_string(j) + "_" +
                                       std::to_string(k);
                this->var_f[i + n * j + k * n * n].setName(var_name.c_str());
                /*				if (k == r) {
                                    x[i][j][k] = 0;
                                }*/
            }
        }
    }

    std::cout << "[INFO] Adicionando variáveis x" << std::endl;
    this->var_x = IloNumVarArray(this->env, n * n);
    for (auto i = 0u; i < n; i++) {
        for (auto j = 0u; j < n; j++) {
            if (this->relaxed) {
                this->var_x[n * i + j] = IloNumVar(this->env, 0, 1, ILOFLOAT);
            } else {
                this->var_x[n * i + j] = IloNumVar(this->env, 0, 1, ILOBOOL);
            }
            std::string var_name =
                "arc_" + std::to_string(i) + "_" + std::to_string(j);
            this->var_x[n * i + j].setName(var_name.c_str());
        }
    }
}

void MultiFlow::add_objective_function() {
    auto n = this->instance.num_vertices;
    IloExpr cost_sum(this->env);
    for (const auto e : boost::make_iterator_range(
             boost::edges(this->instance.covering_graph))) {
        auto i = this->instance.covering_graph[e].source_id;
        auto j = this->instance.covering_graph[e].target_id;
        cost_sum += this->var_x[n * i + j];
    }
    this->cplex_model.add(IloMaximize(this->env, cost_sum, "cost_sum"));
}

void MultiFlow::add_constraints() {
    auto r = this->instance.input_graph[this->instance.root].id;
    auto n = this->instance.num_vertices;

    std::cout << "[INFO] Adicionando restrições de entrada de fluxo"
              << std::endl;
    //	constraints #2
    for (auto v : boost::make_iterator_range(
             boost::vertices(this->instance.input_graph))) {
        if (not this->instance.input_graph[v].is_root) {
            IloExpr in_flow_sum_j(this->env);
            auto j = this->instance.input_graph[v].id;
            my_graph::digraph::in_edge_iterator in_begin, in_end;
            for (boost::tie(in_begin, in_end) =
                     boost::in_edges(v, this->instance.input_graph);
                 in_begin != in_end; ++in_begin) {
                auto u = boost::source(*in_begin, this->instance.input_graph);
                auto i = this->instance.input_graph[u].id;
                in_flow_sum_j += this->var_f[i + n * j + j * n * n];
            }
            this->cplex_model.add(in_flow_sum_j == 1);
        }
    }

    std::cout << "[INFO] Adicionando restrições de saída de fluxo" << std::endl;
    //	constraints #2.5
    for (auto v : boost::make_iterator_range(
             boost::vertices(this->instance.input_graph))) {
        if (not this->instance.input_graph[v].is_root) {
            IloExpr out_flow_sum_j(this->env);
            auto j = this->instance.input_graph[v].id;
            my_graph::digraph::out_edge_iterator in_begin, in_end;
            for (boost::tie(in_begin, in_end) =
                     boost::out_edges(v, this->instance.input_graph);
                 in_begin != in_end; ++in_begin) {
                auto u = boost::target(*in_begin, this->instance.input_graph);
                auto i = this->instance.input_graph[u].id;
                out_flow_sum_j += this->var_f[j + n * i + j * n * n];
            }
            this->cplex_model.add(out_flow_sum_j == 0);
        }
    }

    std::cout << "[INFO] Adicionando restrições de balanceamento de fluxo"
              << std::endl;
    //	constraints #3
    for (auto v : boost::make_iterator_range(
             boost::vertices(this->instance.input_graph))) {
        auto j = this->instance.input_graph[v].id;
        for (auto k = 0u; k < n; k++) {
            if (k != r and k != j and j != r) {
                IloExpr in_flow_sum_k(this->env);
                IloExpr out_flow_sum_k(this->env);

                my_graph::digraph::in_edge_iterator in_begin, in_end;
                for (boost::tie(in_begin, in_end) =
                         boost::in_edges(v, this->instance.input_graph);
                     in_begin != in_end; ++in_begin) {
                    auto u =
                        boost::source(*in_begin, this->instance.input_graph);
                    auto i = this->instance.input_graph[u].id;
                    in_flow_sum_k += this->var_f[i + n * j + n * n * k];
                }

                my_graph::digraph::out_edge_iterator out_begin, out_end;
                for (boost::tie(out_begin, out_end) =
                         boost::out_edges(v, this->instance.input_graph);
                     out_begin != out_end; ++out_begin) {
                    auto u =
                        boost::target(*out_begin, this->instance.input_graph);
                    auto i = this->instance.input_graph[u].id;

                    out_flow_sum_k += this->var_f[j + i * n + k * n * n];
                }
                this->cplex_model.add(in_flow_sum_k == out_flow_sum_k);
            }
        }
    }

    std::cout << "[INFO] Relacionando variáveis x e f" << std::endl;
    //	constraints #6
    for (auto e :
         boost::make_iterator_range(boost::edges(this->instance.input_graph))) {
        auto i = this->instance.input_graph[e].source_id;
        auto j = this->instance.input_graph[e].target_id;
        for (auto k = 0u; k < n; k++) {
            if (k != r)
                this->cplex_model.add(this->var_f[i + n * j + n * n * k] <=
                                      this->var_x[n * i + j]);
        }
    }

    //	constraints #4
    for (auto v : boost::make_iterator_range(
             boost::vertices(this->instance.input_graph))) {
        if (not this->instance.input_graph[v].is_root) {
            IloExpr source_vertices_sum(this->env);
            auto j = this->instance.input_graph[v].id;
            my_graph::digraph::in_edge_iterator in_begin, in_end;
            for (boost::tie(in_begin, in_end) =
                     boost::in_edges(v, this->instance.input_graph);
                 in_begin != in_end; ++in_begin) {
                auto u = boost::source(*in_begin, this->instance.input_graph);
                auto i = this->instance.input_graph[u].id;
                source_vertices_sum += this->var_x[n * i + j];
            }
            this->cplex_model.add(source_vertices_sum == 1);
        }
    }

    //	constraints #5
    for (auto e : boost::make_iterator_range(
             boost::edges(this->instance.covering_graph))) {
        auto v = boost::source(e, this->instance.input_graph);
        auto j = this->instance.input_graph[v].id;
        if (j != r) {
            IloExpr reaches_k(this->env);
            auto k = this->instance.input_graph[e].target_id;
            my_graph::digraph::in_edge_iterator in_begin, in_end;
            for (boost::tie(in_begin, in_end) =
                     boost::in_edges(v, this->instance.input_graph);
                 in_begin != in_end; ++in_begin) {
                auto u = boost::source(*in_begin, this->instance.input_graph);
                auto i = this->instance.input_graph[u].id;
                reaches_k += this->var_f[i + n * j + n * n * k];
            }
            this->cplex_model.add(reaches_k == 1);
        }
    }
}

void MultiFlow::extract_solution() {
    auto solution = my_graph::digraph();
    auto n = this->instance.num_vertices;
    double num_jumps = n - 1 - this->cplex_solver.getObjValue();

    std::cout << "[INFO] Número de saltos: " << num_jumps << std::endl;
    std::cout << "[INFO] Extraindo valores das variáveis" << std::endl;

    for (auto v : boost::make_iterator_range(
             boost::vertices(this->instance.input_graph))) {
        boost::add_vertex(this->instance.input_graph[v], solution);
    }
    //		add edges

    for (auto const& e :
         boost::make_iterator_range(boost::edges(this->instance.input_graph))) {
        auto i = this->instance.input_graph[e].source_id;
        auto j = this->instance.input_graph[e].target_id;
        if (this->cplex_solver.isExtracted(this->var_x[n * i + j])) {
            if (this->cplex_solver.getValue(this->var_x[n * i + j]) > 1e-6) {
                boost::add_edge(i, j, solution);
                std::cout << i << "->" << j
                          << " [label=" << this->instance.input_graph[e].type
                          << ",value="
                          << this->cplex_solver.getValue(this->var_x[n * i + j])
                          << "];\n";
                // std::cout << n * i + j << ',' << i + 1 << ", " << j + 1
                // << " " << this->cplex_solver.getValue(this->cplex_model.y[n *
                // i + j]) << " " << this->instance.input_graph[e].type <<
                // std::endl;
            }
        } else {
            std::cout << "[INFO] Variável não encontrada x[" << i << "," << j
                      << "]" << std::endl;
        }
    }

    auto tc_solution = my_graph::digraph();

    std::map<my_graph::vertex, my_graph::vertex> g_to_tc;
    std::vector<std::size_t> id_map(boost::num_vertices(solution));
    std::iota(id_map.begin(), id_map.end(), 0u);

    boost::transitive_closure(solution, tc_solution,
                              boost::make_assoc_property_map(g_to_tc),
                              id_map.data());

    for (auto& e : g_to_tc) tc_solution[e.second] = solution[e.first];

    for (auto const& e :
         boost::make_iterator_range(boost::edges(this->instance.order_graph))) {
        auto i = this->instance.input_graph[e].source_id;
        auto j = this->instance.input_graph[e].target_id;
        if (not boost::edge(i, j, tc_solution).second) {
            std::cout << "\n === TÁ ERRADO ===" << std::endl;
        }
    }
}

}  // namespace optimizer
