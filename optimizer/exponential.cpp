/*
 * model.cpp
 *
 *  Created on: 19 de abr de 2021
 *      Author: evellyn
 */

#include "exponential.h"

#include "base/instance.h"
#include "utils/const.h"
#include "callback/add_min_cuts.h"


namespace optimizer {

Exponential::Exponential(ajns::Instance& _instance)
    : Optimizer(_instance, EXPONENTIAL, false) {}

Exponential::Exponential(ajns::Instance& _instance, bool _relaxed, bool _with_cutset_constraints, SolverParameters& _parameters)
    : Optimizer(_instance, EXPONENTIAL, _relaxed, _parameters) {
        this->with_cutset_constraints = _with_cutset_constraints;
    }

void Exponential::add_variables() {
    x.resize(this->instance.num_edges);
    for (size_t i = 0; i < this->instance.num_edges; ++i) {
        x[i] = gurobi_model->addVar(0.0, 1.0, 0.0, this->relaxed ? GRB_CONTINUOUS : GRB_BINARY);
        std::string name = "x_" + std::to_string(i);
        x[i].set(GRB_StringAttr_VarName, name);
    }
}

void Exponential::add_constraints() {
    add_number_of_edges_constraints();
    add_limit_indegree_constraints();
    if (this->with_cutset_constraints) {
        add_cutset_constraints();
    }
    // add_out_edges_constraints();
    fix_arcs_value();
    add_bidirected_constraints();
}

void Exponential::add_bidirected_constraints() {
    for (auto p : instance.artificial_arcs_pair) {
        gurobi_model->addConstr(x[p.first] + x[p.second] <= 1);
    }
}

void Exponential::add_objective_function() {
    GRBLinExpr minimize_jumps = 0;
    std::cout << instance.num_edges << std::endl;
    for (const auto& e :
         boost::make_iterator_range(boost::edges(instance.input_graph))) {
        if (instance.input_graph[e].type == my_graph::ARTIFICIAL) {
            minimize_jumps += x[instance.input_graph[e].id];
        }
    }
    gurobi_model->setObjective(minimize_jumps, GRB_MINIMIZE);
}

void Exponential::add_limit_indegree_constraints() {
    size_t n = instance.num_vertices;
    std::vector<GRBLinExpr> indegree_limits_constraint(n, GRBLinExpr());
    std::vector<bool> lhs_expr(n, false);
    for (const auto& e :
         boost::make_iterator_range(boost::edges(instance.input_graph))) {
        auto target_node = instance.input_graph[e].target_id;
        indegree_limits_constraint[target_node] += x[instance.input_graph[e].id];
        lhs_expr[target_node] = true;
    }
    for (size_t i = 0; i < n; i++) {
        if (lhs_expr[i]) gurobi_model->addConstr(indegree_limits_constraint[i] <= 1);
    }
}

void Exponential::add_number_of_edges_constraints() {
    GRBLinExpr number_of_arcs_selected = 0;
    for (size_t i = 0; i < instance.num_edges; i++) {
        number_of_arcs_selected += x[i];
    }
    gurobi_model->addConstr(number_of_arcs_selected == instance.num_vertices - 1);
}

void Exponential::add_out_edges_constraints() {
    my_graph::out_edge_itr ei, ei_end;
    for (auto v :
         boost::make_iterator_range(boost::vertices(instance.input_graph))) {
        if (boost::out_degree(v, instance.covering_graph) > 0) {
            GRBLinExpr out_edges_exp = 0;
            for (boost::tie(ei, ei_end) = out_edges(v, instance.input_graph);
                 ei != ei_end; ++ei) {
                out_edges_exp += x[instance.input_graph[*ei].id];
            }
            gurobi_model->addConstr(out_edges_exp >= 1);
        }
    }
}

void Exponential::add_cutset_constraints() {
    for (auto powerset = 1u; powerset < std::pow(2, instance.num_vertices); powerset++) {
        GRBLinExpr extension_constraint = 0;
        bool add_expr = false;
        for (const auto& e :
             boost::make_iterator_range(boost::edges(instance.input_graph))) {
            auto head = instance.input_graph[e].source_id;
            auto tail = instance.input_graph[e].target_id;

            if (in_the_set(powerset, head) && !in_the_set(powerset, tail)) {
                if (instance.input_graph[e].type == my_graph::ORIGINAL) {
                    add_expr = true;
                }
                extension_constraint += x[instance.input_graph[e].id];
            }
        }
        if (add_expr) gurobi_model->addConstr(extension_constraint >= 1);
    }
}

void Exponential::fix_arcs_value() {
    auto count = 0u;
    for (const auto& e :
         boost::make_iterator_range(boost::edges(instance.input_graph))) {
        if (instance.input_graph[e].value_set) {
            auto id = instance.input_graph[e].id;
            if (instance.input_graph[e].value == 1u) {
                x[id].set(GRB_DoubleAttr_LB, 1.0);
                x[id].set(GRB_DoubleAttr_UB, 1.0);
            } else {
                x[id].set(GRB_DoubleAttr_LB, 0.0);
                x[id].set(GRB_DoubleAttr_UB, 0.0);
            }
            count++;
        }
    }
    std::cout << "\n=> SET VALUE FOR " << count << " EDGES\n";
}

bool Exponential::in_the_set(unsigned _set, std::size_t _element) {
    return (_set & (1 << _element));
}

std::vector<GRBVar> Exponential::get_variables_x() { return this->x; }

std::vector<GRBVar> Exponential::get_variables(int which) { return this->x; }

void Exponential::extract_solution() {}

void Exponential::add_callbacks() {
    // std::cout << "Adding callbacks..." << std::endl;
    // GRBVar* x_array = this->x.data();
    // AddMinCutsCallback* min_cuts_cb = new AddMinCutsCallback(this->instance, this->x);
    // this->gurobi_model->setCallback(min_cuts_cb);
    // std::cout << "Callbacks added!" << std::endl;
}

void Exponential::run() {
    Optimizer::run();
}

}  // namespace optimizer