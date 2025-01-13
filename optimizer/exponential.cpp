/*
 * model.cpp
 *
 *  Created on: 19 de abr de 2021
 *      Author: evellyn
 */

#include "exponential.h"

#include "base/instance.h"
#include "utils/const.h"

namespace optimizer {

Exponential::Exponential(ajns::Instance& _instance)
    : Optimizer(_instance, EXPONENTIAL, false) {}

Exponential::Exponential(ajns::Instance& _instance, bool _relaxed)
    : Optimizer(_instance, EXPONENTIAL, _relaxed) {}

void Exponential::add_variables() {
    this->x = IloBoolVarArray(this->env, this->instance.num_edges);
}

void Exponential::add_constraints() {
    if (!this->relaxed) {
        add_number_of_edges_constraints();
        add_limit_indegree_constraints();
        add_cutset_constraints();
    } else {
        add_number_of_edges_constraints();
        add_limit_indegree_constraints();
    }
    // add_out_edges_constraints();
    fix_arcs_value();
    add_bidirected_constraints();
}

void Exponential::add_bidirected_constraints() {
    for (auto p : instance.artificial_arcs_pair) {
        IloExpr one_bidirected(env);
        one_bidirected = x[p.first] + x[p.second];
        cplex_model.add(one_bidirected <= 1);
        one_bidirected.end();
    }
}

void Exponential::add_objective_function() {
    IloExpr minimize_jumps(env);
    std::cout << instance.num_edges << std::endl;
    for (const auto& e :
         boost::make_iterator_range(boost::edges(instance.input_graph))) {
        if (instance.input_graph[e].type == my_graph::ARTIFICIAL) {
            minimize_jumps += x[instance.input_graph[e].id];
        }
    }
    this->cplex_model.add(IloMinimize(env, minimize_jumps));
    minimize_jumps.end();
}

void Exponential::add_limit_indegree_constraints() {
    IloExprArray indegree_limits_constraint(env, instance.num_vertices);
    std::vector<bool> lhs_expr(instance.num_vertices, false);
    for (auto i = 0u; i < instance.num_vertices; i++)
        indegree_limits_constraint[i] = IloExpr(env);
    for (const auto& e :
         boost::make_iterator_range(boost::edges(instance.input_graph))) {
        auto target_node = instance.input_graph[e].target_id;
        indegree_limits_constraint[target_node] +=
            x[instance.input_graph[e].id];
        lhs_expr[target_node] = true;
    }
    for (auto i = 0u; i < instance.num_vertices; i++) {
        if (lhs_expr[i]) cplex_model.add(indegree_limits_constraint[i] <= 1);
        indegree_limits_constraint[i].end();
    }
    indegree_limits_constraint.end();
}

void Exponential::add_number_of_edges_constraints() {
    IloExpr number_of_arcs_selected(env);
    for (auto i = 0u; i < instance.num_edges; i++) {
        number_of_arcs_selected += x[i];
    }
    cplex_model.add(number_of_arcs_selected == instance.num_vertices - 1);
    number_of_arcs_selected.end();
}

void Exponential::add_out_edges_constraints() {
    my_graph::out_edge_itr ei, ei_end;
    for (auto v :
         boost::make_iterator_range(boost::vertices(instance.input_graph))) {
        if (boost::out_degree(v, instance.covering_graph) > 0) {
            auto vertex_set = std::set<my_graph::vertex>();
            IloExpr out_edges_exp(env);
            for (boost::tie(ei, ei_end) = out_edges(v, instance.input_graph);
                 ei != ei_end; ++ei) {
                out_edges_exp += x[instance.input_graph[*ei].id];
            }
            cplex_model.add(out_edges_exp >= 1);
        }
    }
}

void Exponential::add_cutset_constraints() {
    for (auto powerset = 1; powerset < std::pow(2, instance.num_vertices);
         powerset++) {
        IloExpr extension_constraint(env);
        auto add_expr = false;
        for (const auto& e :
             boost::make_iterator_range(boost::edges(instance.input_graph))) {
            auto head = instance.input_graph[e].source_id;
            auto tail = instance.input_graph[e].target_id;

            if (in_the_set(powerset, head) && not in_the_set(powerset, tail)) {
                if (instance.input_graph[e].type == my_graph::ORIGINAL) {
                    add_expr = true;
                }
                extension_constraint += x[instance.input_graph[e].id];
            }
        }
        if (add_expr) cplex_model.add(extension_constraint >= 1);
        extension_constraint.end();
    }
}

void Exponential::fix_arcs_value() {
    auto count = 0u;
    for (const auto& e :
         boost::make_iterator_range(boost::edges(instance.input_graph))) {
        if (instance.input_graph[e].value_set) {
            auto id = instance.input_graph[e].id;
            // IloExpr exp(env);
            // exp += this->x[id];
            if (instance.input_graph[e].value == 1u) {
                this->x[id].setBounds(1, 1);
                /*cplex_model.add(exp >= 1);
                cplex_model.add(exp <= 1);*/
            } else {
                this->x[id].setBounds(0, 0);
                /*cplex_model.add(exp <= 0);
                cplex_model.add(exp >= 0);*/
            }
            // exp.end();
            count++;
        }
    }
    std::cout << "\n=> SET VALUE FOR " << count << " EDGES\n";
}

bool Exponential::in_the_set(unsigned _set, std::size_t _element) {
    return (_set & (1 << _element));
}

IloBoolVarArray Exponential::get_variables_x() { return this->x; }

auto Exponential::get_variables(int which) { return this->x; }

void Exponential::extract_solution() {}

// ExponentialModel::~ExponentialModel() {
////	env.end();
////	cplex_model.end();
//}

}  // namespace optimizer
