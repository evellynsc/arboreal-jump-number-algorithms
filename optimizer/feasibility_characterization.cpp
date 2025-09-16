#include "feasibility_characterization.h"

#include <cmath>
#include "optimizer.h"
#include "utils/const.h"

namespace optimizer {

FeasibilityCharacterization::FeasibilityCharacterization(ajns::Instance& _instance, bool _relaxed)
    : Optimizer(_instance, CHARACTERIZATION, _relaxed) {}

FeasibilityCharacterization::FeasibilityCharacterization(ajns::Instance& _instance, bool _relaxed, int _num_jumps)
    : Optimizer(_instance, CHARACTERIZATION, _relaxed) {
    this->num_jumps = _num_jumps;
}

void FeasibilityCharacterization::set_num_jumps(int _num_jumps) {
    this->num_jumps = _num_jumps;
}

void FeasibilityCharacterization::restart_model() {
    delete this->gurobi_model;
    delete this->env;
    this->env = new GRBEnv();
    this->gurobi_model = new GRBModel(*this->env);
    this->solution = nullptr;
    x.clear();
    r.clear();
    f.clear();
    g.clear();
    a.clear();
    h.clear();
    w.clear();
}

std::vector<GRBVar> FeasibilityCharacterization::get_x_variables() { return x; }
std::vector<std::vector<GRBVar>> FeasibilityCharacterization::get_a_variables() { return a; }
std::vector<GRBVar> FeasibilityCharacterization::get_r_variables() { return r; }
int FeasibilityCharacterization::get_num_jumps() { return this->num_jumps; }
std::vector<GRBVar> FeasibilityCharacterization::get_f_variables() { return f; }
std::vector<GRBVar> FeasibilityCharacterization::get_g_variables() { return g; }

void FeasibilityCharacterization::add_variables() {
    int m = this->instance.num_vertices * this->instance.num_vertices;
    int s1 = this->num_jumps + 1;
    int ns = this->instance.num_vertices * s1;
    int s2 = s1 * s1;

    index_parser_ns = std::vector<std::vector<int>>(this->instance.num_vertices);
    index_parser_m = std::vector<std::vector<int>>(this->instance.num_vertices);
    index_parser_s2 = std::vector<std::vector<int>>(s1);

    for (int i = 0; i < this->instance.num_vertices; i++) {
        index_parser_m[i] = std::vector<int>(this->instance.num_vertices);
        for (int j = 0; j < this->instance.num_vertices; j++) {
            index_parser_m[i][j] = i * this->instance.num_vertices + j;
        }
    }

    for (int i = 0; i < this->instance.num_vertices; i++) {
        index_parser_ns[i] = std::vector<int>(s1);
        for (int t = 0; t <= this->num_jumps; t++) {
            index_parser_ns[i][t] = t * this->instance.num_vertices + i;
        }
    }

    for (int u = 0; u <= this->num_jumps; u++) {
        index_parser_s2[u] = std::vector<int>(s1);
        for (int t = 0; t <= this->num_jumps; t++) {
            index_parser_s2[u][t] = t * s1 + u;
        }
    }

    x.resize(ns);
    r.resize(ns);
    f.resize(ns);
    g.resize(ns);

    for (int idx = 0; idx < ns; idx++) {
        x[idx] = gurobi_model->addVar(0.0, 1.0, 0.0, this->relaxed ? GRB_CONTINUOUS : GRB_BINARY);
        r[idx] = gurobi_model->addVar(0.0, 1.0, 0.0, this->relaxed ? GRB_CONTINUOUS : GRB_BINARY);
        f[idx] = gurobi_model->addVar(0.0, 1.0, 0.0, this->relaxed ? GRB_CONTINUOUS : GRB_BINARY);
        g[idx] = gurobi_model->addVar(0.0, 1.0, 0.0, this->relaxed ? GRB_CONTINUOUS : GRB_BINARY);
    }

    a.resize(m, std::vector<GRBVar>(s1));
    h.resize(m, std::vector<GRBVar>(s2));
    w.resize(ns, std::vector<GRBVar>(s1));

    for (int i = 0; i < m; i++) {
        for (int t = 0; t < s1; t++) {
            a[i][t] = gurobi_model->addVar(0.0, 1.0, 0.0, this->relaxed ? GRB_CONTINUOUS : GRB_BINARY);
        }
        for (int tu = 0; tu < s2; tu++) {
            h[i][tu] = gurobi_model->addVar(0.0, 1.0, 0.0, this->relaxed ? GRB_CONTINUOUS : GRB_BINARY);
        }
    }
    for (int i = 0; i < ns; i++) {
        for (int u = 0; u < s1; u++) {
            w[i][u] = gurobi_model->addVar(0.0, 1.0, 0.0, this->relaxed ? GRB_CONTINUOUS : GRB_BINARY);
        }
    }

    // Naming variables
    for (int i = 0; i < this->instance.num_vertices; i++) {
        for (int j = 0; j < this->instance.num_vertices; j++) {
            for (int t = 0; t < s1; t++) {
                std::string name = "a_" + std::to_string(i) + "_" +
                                   std::to_string(j) + "_" + std::to_string(t);
                auto idx_ij = index_parser_m[i][j];
                a[idx_ij][t].set(GRB_StringAttr_VarName, name);
            }
        }
    }

    for (int i = 0; i < this->instance.num_vertices; i++) {
        for (int j = 0; j < this->instance.num_vertices; j++) {
            for (int t = 0; t < s1; t++) {
                for (int u = 0; u < s1; u++) {
                    std::string name =
                        "h_" + std::to_string(i) + "_" + std::to_string(j) +
                        "_" + std::to_string(t) + "_" + std::to_string(u);
                    auto idx_ij = index_parser_m[i][j];
                    auto idx_tu = index_parser_s2[t][u];
                    h[idx_ij][idx_tu].set(GRB_StringAttr_VarName, name);
                }
            }
        }
    }

    for (int i = 0; i < this->instance.num_vertices; i++) {
        for (int t = 0; t < s1; t++) {
            std::string name =
                "x_" + std::to_string(i) + "_" + std::to_string(t);
            auto idx = index_parser_ns[i][t];
            x[idx].set(GRB_StringAttr_VarName, name);

            name = "r_" + std::to_string(i) + "_" + std::to_string(t);
            r[idx].set(GRB_StringAttr_VarName, name);

            name = "f_" + std::to_string(i) + "_" + std::to_string(t);
            f[idx].set(GRB_StringAttr_VarName, name);

            name = "g_" + std::to_string(i) + "_" + std::to_string(t);
            g[idx].set(GRB_StringAttr_VarName, name);

            for (int u = 0; u < s1; u++) {
                name = "w_" + std::to_string(i) + "_" + std::to_string(t) +
                       "_" + std::to_string(u);
                w[idx][u].set(GRB_StringAttr_VarName, name);
            }
        }
    }
}

void FeasibilityCharacterization::add_constraints() {
    // sum_{t} x_{i,t} == 1 for all i
    for (int i = 0; i < this->instance.num_vertices; i++) {
        GRBLinExpr sum_x = 0;
        for (int t = 0; t <= this->num_jumps; t++) {
            auto idx = index_parser_ns[i][t];
            sum_x += x[idx];
        }
        gurobi_model->addConstr(sum_x == 1);
    }

    // sum_{i} x_{i,t} >= 1 for all t
    // for (int t = 0; t <= this->num_jumps; t++) {
    //     GRBLinExpr sum_x = 0;
    //     for (int i = 0; i < this->instance.num_vertices; i++) {
    //         auto idx = index_parser_ns[i][t];
    //         sum_x += x[idx];
    //     }
    //     gurobi_model->addConstr(sum_x >= 1);
    // }

    // Constraints (2)-(5)
    for (auto const e : boost::make_iterator_range(
             boost::edges(this->instance.covering_graph))) {
        auto i = this->instance.covering_graph[e].source_id;
        auto j = this->instance.covering_graph[e].target_id;
        for (int t = 0; t <= this->num_jumps; t++) {
            auto idx_it = index_parser_ns[i][t];
            auto idx_jt = index_parser_ns[j][t];
            auto idx_m = index_parser_m[i][j];
            gurobi_model->addConstr(x[idx_it] >= a[idx_m][t]);
            gurobi_model->addConstr(x[idx_jt] >= a[idx_m][t]);
            gurobi_model->addConstr(a[idx_m][t] - x[idx_jt] - x[idx_it] >= -1);

            // GRBLinExpr sum_x = 0;
            // bool added = false;
            // for (int u = t + 1; u <= this->num_jumps - 1; u++) {
            //     sum_x += x[index_parser_ns[i][u]];
            //     added = true;
            // }
            // if (added) {
            //     sum_x += x[idx_jt];
            //     gurobi_model->addConstr(sum_x <= 1);
            // }
        }
    }

    // r_{root,0} == 1
    gurobi_model->addConstr(r[index_parser_ns[this->instance.root][0]] == 1);

    // sum_{i} r_{i,t} == 1 for all t
    for (int t = 0; t <= this->num_jumps; t++) {
        GRBLinExpr sum_r = 0;
        for (int i = 0; i < this->instance.num_vertices; i++) {
            auto idx = index_parser_ns[i][t];
            sum_r += r[idx];
        }
        gurobi_model->addConstr(sum_r == 1);
    }

    // Constraint (6)
    for (int i = 0; i < this->instance.num_vertices; i++) {
        for (int j = 0; j < this->instance.num_vertices; j++) {
            if (this->instance.exist_edge_in_covering_graph(i, j)) {
                for (int u = 0; u <= this->num_jumps; u++) {
                    GRBLinExpr sum_x = 0;
                    bool add = false;
                    for (int t = u + 1; t <= this->num_jumps; t++) {
                        sum_x += x[index_parser_ns[i][t]];
                        add = true;
                    }
                    if (add) {
                        sum_x += x[index_parser_ns[j][u]];
                        gurobi_model->addConstr(sum_x <= 1);
                    }
                }
            }
        }
    }

    // x_{jt} >= r_{jt}
    for (int i = 0; i < this->instance.num_vertices; i++) {
        for (int t = 0; t <= this->num_jumps; t++) {
            auto idx = index_parser_ns[i][t];
            gurobi_model->addConstr(x[idx] >= r[idx]);
        }
    }

    // sum_{t} r_{jt} + sum_{i} sum_{t} a_{ijt} == 1 for j != root
    for (int j = 0; j < this->instance.num_vertices; j++) {
        if (j != this->instance.root) {
            GRBLinExpr sum_a = 0;
            GRBLinExpr sum_r = 0;
            for (int t = 0; t <= this->num_jumps; t++) {
                if (this->instance.covering_predecessors.at(j).size() > 0) {
                    for (auto i : this->instance.covering_predecessors.at(j)) {
                        auto idx_m = index_parser_m[i][j];
                        sum_a += a[idx_m][t];
                    }
                }
                auto idx_jt = index_parser_ns[j][t];
                sum_r += r[idx_jt];
            }
            gurobi_model->addConstr(sum_r + sum_a == 1);
        }
    }

    // // sum_{t} a_{ijt} <= 1 for j != root
    // for (int j = 0; j < this->instance.num_vertices; j++) {
    //     if (j != this->instance.root) {
    //         GRBLinExpr sum_a = 0;
    //         for (int t = 0; t <= this->num_jumps; t++) {
    //             for (int i : this->instance.covering_predecessors.at(j)) {
    //                 auto idx_m = index_parser_m[i][j];
    //                 sum_a += a[idx_m][t];
    //             }
    //         }
    //         gurobi_model->addConstr(sum_a <= 1);
    //     }
    // }

    // sum_{i} f_{it} == 1 for t > 0
    for (int t = 1; t <= this->num_jumps; t++) {
        GRBLinExpr sum_f = 0;
        for (int i = 0; i < this->instance.num_vertices; i++) {
            auto idx_it = index_parser_ns[i][t];
            sum_f += f[idx_it];
        }
        gurobi_model->addConstr(sum_f == 1);
    }

    // f_{iu} <= sum_{t < u} x_{it}
    for (int u = 1; u <= this->num_jumps; u++) {
        for (int i = 0; i < this->instance.num_vertices; i++) {
            GRBLinExpr sum_x = 0;
            for (int t = 0; t < u; t++) {
                auto idx_it = index_parser_ns[i][t];
                sum_x += x[idx_it];
            }
            auto idx_iu = index_parser_ns[i][u];
            gurobi_model->addConstr(f[idx_iu] <= sum_x);
        }
    }

    // f_{it} + r_{jt} + d_{ij} <= 2
    for (int i = 0; i < this->instance.num_vertices; i++) {
        for (int j = 0; j < this->instance.num_vertices; j++) {
            for (int t = 1; t <= this->num_jumps; t++) {
                auto idx_it = index_parser_ns[i][t];
                auto idx_jt = index_parser_ns[j][t];
                int d_ij = this->instance.exist_edge_in_order_graph(i, j);
                gurobi_model->addConstr(f[idx_it] + r[idx_jt] + d_ij <= 2);
            }
        }
    }

    // h_{ijtu} <= f_{ju}, h_{ijtu} <= x_{jt}, h_{ijtu} <= g_{it}, h_{ijtu} >= f_{ju} + x_{jt} + g_{it} - 2
    for (int i = 0; i < this->instance.num_vertices; i++) {
        for (int j = 0; j < this->instance.num_vertices; j++) {
            for (int t = 0; t <= this->num_jumps; t++) {
                for (int u = t + 1; u <= this->num_jumps; u++) {
                    auto idx_ij = index_parser_m[i][j];
                    auto idx_tu = index_parser_s2[t][u];
                    auto idx_ju = index_parser_ns[j][u];
                    auto idx_jt = index_parser_ns[j][t];
                    auto idx_it = index_parser_ns[i][t];

                    gurobi_model->addConstr(h[idx_ij][idx_tu] <= f[idx_ju]);
                    gurobi_model->addConstr(h[idx_ij][idx_tu] <= x[idx_jt]);
                    gurobi_model->addConstr(h[idx_ij][idx_tu] <= g[idx_it]);
                    gurobi_model->addConstr(h[idx_ij][idx_tu] >= f[idx_ju] + x[idx_jt] + g[idx_it] - 2);
                }
            }
        }
    }

    // g_{iu} == f_{iu} + sum_{j, t < u} h_{ijtu}
    for (int i = 0; i < this->instance.num_vertices; i++) {
        for (int u = 1; u <= this->num_jumps; u++) {
            GRBLinExpr sum_h = 0;
            for (int j = 0; j < this->instance.num_vertices; j++) {
                for (int t = 0; t < u; t++) {
                    auto idx_ij = index_parser_m[i][j];
                    auto idx_tu = index_parser_s2[t][u];
                    sum_h += h[idx_ij][idx_tu];
                }
            }
            auto idx_iu = index_parser_ns[i][u];
            gurobi_model->addConstr(g[idx_iu] == f[idx_iu] + sum_h);
        }
    }

    // w_{itu} <= x_{it}, w_{itu} <= g_{iu}, w_{itu} >= x_{it} + g_{iu} - 1
    for (int i = 0; i < this->instance.num_vertices; i++) {
        for (int t = 0; t <= this->num_jumps; t++) {
            for (int u = 0; u <= this->num_jumps; u++) {
                if (t != u) {
                    auto idx_it = index_parser_ns[i][t];
                    auto idx_iu = index_parser_ns[i][u];
                    gurobi_model->addConstr(w[idx_it][u] <= x[idx_it]);
                    gurobi_model->addConstr(w[idx_it][u] <= g[idx_iu]);
                    gurobi_model->addConstr(w[idx_it][u] >= x[idx_it] + g[idx_iu] - 1);
                }
            }
        }
    }

    // x_{it} + x_{ju} <= 1 + sum_{k} w_{ktu}
    for (auto const e : boost::make_iterator_range(
             boost::edges(this->instance.covering_graph))) {
        auto i = this->instance.covering_graph[e].source_id;
        auto j = this->instance.covering_graph[e].target_id;
        for (int t = 0; t <= this->num_jumps; t++) {
            for (int u = 0; u <= this->num_jumps; u++) {
                if (t != u) {
                    GRBLinExpr sum_q = 0;
                    auto idx_kt = index_parser_ns[i][t];
                    sum_q += w[idx_kt][u];
                    for (int k : this->instance.sucessors.at(i)) {
                        idx_kt = index_parser_ns[k][t];
                        sum_q += w[idx_kt][u];
                    }
                    auto idx_it = index_parser_ns[i][t];
                    auto idx_ju = index_parser_ns[j][u];
                    gurobi_model->addConstr(x[idx_it] + x[idx_ju] <= 1 + sum_q);
                }
            }
        }
    }

    // g_{i0} == 0
    for (int i = 0; i < this->instance.num_vertices; i++) {
        auto idx_i0 = index_parser_ns[i][0];
        gurobi_model->addConstr(g[idx_i0] == 0);
    }
}

void FeasibilityCharacterization::add_objective_function() {
    GRBLinExpr number_of_parts = 0;
    for (int i = 0; i < this->instance.num_vertices; i++) {
        for (int t = 0; t <= this->num_jumps; t++) {
            auto idx_it = index_parser_ns[i][t];
            number_of_parts += r[idx_it];
        }
    }
    gurobi_model->setObjective(number_of_parts, GRB_MINIMIZE);
}

void FeasibilityCharacterization::extract_solution() {
    this->solution = new Solution(this->num_jumps);
    for (int i = 0; i < this->instance.num_vertices; i++) {
        for (int j = 0; j < this->instance.num_vertices; j++) {
            for (int t = 0; t <= this->num_jumps; t++) {
                auto idx_ij = index_parser_m[i][j];
                if (a[idx_ij][t].get(GRB_DoubleAttr_X) > 1e-6) {
                    solution->add_edge(idx_ij, i, j, false);
                }
            }
        }
    }
    std::map<int, int> r_map;
    for (int i = 0; i < this->instance.num_vertices; i++) {
        for (int t = 0; t <= this->num_jumps; t++) {
            auto idx_it = index_parser_ns[i][t];
            if (r[idx_it].get(GRB_DoubleAttr_X) > 1e-6) {
                r_map[t] = i;
            }
        }
    }

    for (int i = 0; i < this->instance.num_vertices; i++) {
        for (int t = 0; t <= this->num_jumps; t++) {
            auto idx_it = index_parser_ns[i][t];
            if (f[idx_it].get(GRB_DoubleAttr_X) > 1e-6 && i != this->instance.root) {
                solution->add_edge(idx_it, i, r_map[t], true);
            }
        }
    }
}

int FeasibilityCharacterization::idx_ns(int i, int j) { return index_parser_ns[i][j]; }

void FeasibilityCharacterization::run() {
    auto naive_lower_bound = this->instance.get_max_indegree() - 1;

    for (int attempt = naive_lower_bound; attempt <= this->instance.num_vertices; attempt++) {
        std::cout << "===============================================" << std::endl;
        std::cout << "Attempting with num_jumps = " << attempt << std::endl;
        this->num_jumps = attempt;
        Optimizer::run();
        this->metrics->sum_time += this->metrics->solve_time;
        if (this->solved) {
            this->metrics->num_jumps = this->num_jumps;
            break;
        }
        this->restart_model();
        std::cout << "===============================================" << std::endl;
    }
}

}  // namespace optimizer