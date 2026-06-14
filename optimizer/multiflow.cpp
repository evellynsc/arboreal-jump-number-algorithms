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

namespace optimizer
{

    MultiFlow::MultiFlow(ajns::Instance& _instance)
        : Optimizer(_instance, MULTIFLOW, false) {
    }

    MultiFlow::MultiFlow(ajns::Instance& _instance, bool _relaxed)
        : Optimizer(_instance, MULTIFLOW, _relaxed) {
    }

    MultiFlow::MultiFlow(ajns::Instance& _instance, bool _relaxed,
        SolverParameters& _parameters)
        : Optimizer(_instance, MULTIFLOW, _relaxed, _parameters) {
    }

    std::vector<GRBVar> MultiFlow::get_y_variables() { return this->var_x; }

    void MultiFlow::reset_upper_bounds(std::vector<bool> zero_variables) {
        int n = this->instance.num_vertices;
        for (auto e :
            boost::make_iterator_range(boost::edges(this->instance.input_graph))) {
            int i = this->instance.input_graph[e].source_id;
            int j = this->instance.input_graph[e].target_id;
            int idx = n * i + j;
            if (zero_variables[idx]) {
                this->var_x[idx].set(GRB_DoubleAttr_UB, 0.0);
            }
        }
    }

    void MultiFlow::add_variables() {
        int n = this->instance.num_vertices;

        // Add flow variables: var_f[i + n * j + k * n * n]
        this->var_f.resize(n * n * n);
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                for (int k = 0; k < n; k++) {
                    this->var_f[i + n * j + k * n * n] =
                        gurobi_model->addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS);
                    std::string var_name = "flow_" + std::to_string(i) + "_" +
                        std::to_string(j) + "_" +
                        std::to_string(k);
                    this->var_f[i + n * j + k * n * n].set(GRB_StringAttr_VarName, var_name);
                }
            }
        }

        // Add arc selection variables: var_x[n * i + j]
        this->var_x.resize(n * n);
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (this->relaxed) {
                    this->var_x[n * i + j] = gurobi_model->addVar(0, 1, 0, GRB_CONTINUOUS);
                }
                else {
                    this->var_x[n * i + j] = gurobi_model->addVar(0, 1, 0, GRB_BINARY);
                }
                std::string var_name =
                    "arc_" + std::to_string(i) + "_" + std::to_string(j);
                this->var_x[n * i + j].set(GRB_StringAttr_VarName, var_name);
            }
        }
    }

    void MultiFlow::add_objective_function() {
        int n = this->instance.num_vertices;
        GRBLinExpr cost_sum = 0;
        for (const auto e : boost::make_iterator_range(
            boost::edges(this->instance.covering_graph))) {
            auto i = this->instance.covering_graph[e].source_id;
            auto j = this->instance.covering_graph[e].target_id;
            cost_sum += this->var_x[n * i + j];
        }
        gurobi_model->setObjective(cost_sum, GRB_MAXIMIZE);
    }

    void MultiFlow::add_constraints() {
        int r = this->instance.input_graph[this->instance.root].id;
        int n = this->instance.num_vertices;

        // Flow in constraints
        for (auto v : boost::make_iterator_range(
            boost::vertices(this->instance.input_graph))) {
            if (!this->instance.input_graph[v].is_root) {
                GRBLinExpr in_flow_sum_j = 0;
                auto j = this->instance.input_graph[v].id;
                my_graph::digraph::in_edge_iterator in_begin, in_end;
                for (boost::tie(in_begin, in_end) =
                    boost::in_edges(v, this->instance.input_graph);
                    in_begin != in_end; ++in_begin) {
                    auto u = boost::source(*in_begin, this->instance.input_graph);
                    auto i = this->instance.input_graph[u].id;
                    in_flow_sum_j += this->var_f[i + n * j + j * n * n];
                }
                gurobi_model->addConstr(in_flow_sum_j == 1);
            }
        }

        // Flow out constraints
        for (auto v : boost::make_iterator_range(
            boost::vertices(this->instance.input_graph))) {
            if (!this->instance.input_graph[v].is_root) {
                GRBLinExpr out_flow_sum_j = 0;
                auto j = this->instance.input_graph[v].id;
                my_graph::digraph::out_edge_iterator out_begin, out_end;
                for (boost::tie(out_begin, out_end) =
                    boost::out_edges(v, this->instance.input_graph);
                    out_begin != out_end; ++out_begin) {
                    auto u = boost::target(*out_begin, this->instance.input_graph);
                    auto i = this->instance.input_graph[u].id;
                    out_flow_sum_j += this->var_f[j + n * i + j * n * n];
                }
                gurobi_model->addConstr(out_flow_sum_j == 0);
            }
        }

        // Flow balance constraints
        for (auto v : boost::make_iterator_range(
            boost::vertices(this->instance.input_graph))) {
            auto j = this->instance.input_graph[v].id;
            for (int k = 0; k < n; k++) {
                if (k != r && k != j && j != r) {
                    GRBLinExpr in_flow_sum_k = 0;
                    GRBLinExpr out_flow_sum_k = 0;

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
                    gurobi_model->addConstr(in_flow_sum_k == out_flow_sum_k);
                }
            }
        }

        // Relate x and f variables
        for (auto e :
            boost::make_iterator_range(boost::edges(this->instance.input_graph))) {
            auto i = this->instance.input_graph[e].source_id;
            auto j = this->instance.input_graph[e].target_id;
            for (int k = 0; k < n; k++) {
                if (k != r)
                    gurobi_model->addConstr(this->var_f[i + n * j + n * n * k] <=
                        this->var_x[n * i + j]);
            }
        }

        // Source vertices sum constraints
        for (auto v : boost::make_iterator_range(
            boost::vertices(this->instance.input_graph))) {
            if (!this->instance.input_graph[v].is_root) {
                GRBLinExpr source_vertices_sum = 0;
                auto j = this->instance.input_graph[v].id;
                my_graph::digraph::in_edge_iterator in_begin, in_end;
                for (boost::tie(in_begin, in_end) =
                    boost::in_edges(v, this->instance.input_graph);
                    in_begin != in_end; ++in_begin) {
                    auto u = boost::source(*in_begin, this->instance.input_graph);
                    auto i = this->instance.input_graph[u].id;
                    source_vertices_sum += this->var_x[n * i + j];
                }
                gurobi_model->addConstr(source_vertices_sum == 1);
            }
        }

        // Covering graph constraints
        for (auto e : boost::make_iterator_range(
            boost::edges(this->instance.covering_graph))) {
            auto v = boost::source(e, this->instance.input_graph);
            auto j = this->instance.input_graph[v].id;
            if (j != r) {
                GRBLinExpr reaches_k = 0;
                auto k = this->instance.input_graph[e].target_id;
                my_graph::digraph::in_edge_iterator in_begin, in_end;
                for (boost::tie(in_begin, in_end) =
                    boost::in_edges(v, this->instance.input_graph);
                    in_begin != in_end; ++in_begin) {
                    auto u = boost::source(*in_begin, this->instance.input_graph);
                    auto i = this->instance.input_graph[u].id;
                    reaches_k += this->var_f[i + n * j + n * n * k];
                }
                gurobi_model->addConstr(reaches_k == 1);
            }
        }
    }

    void MultiFlow::extract_solution() {
        auto solution = my_graph::digraph();
        int n = this->instance.num_vertices;
        double num_jumps = n - 1 - gurobi_model->get(GRB_DoubleAttr_ObjVal);

        for (auto v : boost::make_iterator_range(
            boost::vertices(this->instance.input_graph))) {
            boost::add_vertex(this->instance.input_graph[v], solution);
        }
        // Add edges
        for (auto const& e :
            boost::make_iterator_range(boost::edges(this->instance.input_graph))) {
            auto i = this->instance.input_graph[e].source_id;
            auto j = this->instance.input_graph[e].target_id;
            if (this->var_x[n * i + j].get(GRB_DoubleAttr_X) > 1e-6) {
                boost::add_edge(i, j, solution);
                std::cout << i << "->" << j
                    << " [label=" << this->instance.input_graph[e].type
                    << ",value="
                    << this->var_x[n * i + j].get(GRB_DoubleAttr_X)
                    << "];\n";
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
            if (!boost::edge(i, j, tc_solution).second) {
                std::cout << "\n === TÁ ERRADO ===" << std::endl;
            }
        }
    }

    void MultiFlow::run() {
        Optimizer::run();
        int n = this->instance.num_vertices;
        this->metrics->num_jumps = n - 1 - gurobi_model->get(GRB_DoubleAttr_ObjVal);
        std::cout << "Number of jumps: " << n << " - " << this->metrics->num_jumps << std::endl;
    }

}  // namespace optimizer