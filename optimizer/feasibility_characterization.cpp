#include "feasibility_characterization.h"

#include <cmath>

#include "ilconcert/iloexpression.h"
#include "optimizer.h"
#include "utils/const.h"

namespace optimizer {

FeasibilityCharacterization::FeasibilityCharacterization(ajns::Instance& _instance, bool _relaxed)
    : Optimizer(_instance, CHARACTERIZATION, _relaxed) {}

FeasibilityCharacterization::FeasibilityCharacterization(ajns::Instance& _instance, bool _relaxed,
                                   int _num_jumps)
    : Optimizer(_instance, CHARACTERIZATION, _relaxed) {
    this->num_jumps = _num_jumps;
}

void FeasibilityCharacterization::set_num_jumps(int _num_jumps) {
    this->num_jumps = _num_jumps;
}

IloNumVarArray FeasibilityCharacterization::get_x_variables() { return x; }

IloArray<IloNumVarArray> FeasibilityCharacterization::get_a_variables() { return a; }

IloNumVarArray FeasibilityCharacterization::get_r_variables() { return r; }

int FeasibilityCharacterization::get_num_jumps() { return this->num_jumps; }

IloNumVarArray FeasibilityCharacterization::get_f_variables() { return f; }

IloNumVarArray FeasibilityCharacterization::get_g_variables() { return g; }

void FeasibilityCharacterization::add_variables() {
    auto m = this->instance.num_vertices * this->instance.num_vertices;

    auto s1 = this->num_jumps + 1u;
    auto ns = this->instance.num_vertices * s1;
    auto s2 = s1 * s1;

    index_parser_ns = vector<vector<int>>(this->instance.num_vertices);
    index_parser_m = vector<vector<int>>(this->instance.num_vertices);
    index_parser_s2 = vector<vector<int>>(s1);

    for (int i = 0; i < this->instance.num_vertices; i++) {
        index_parser_m[i] = vector<int>(this->instance.num_vertices);
        for (int j = 0; j < this->instance.num_vertices; j++) {
            index_parser_m[i][j] = i * this->instance.num_vertices + j;
        }
    }

    for (int i = 0; i < this->instance.num_vertices; i++) {
        index_parser_ns[i] = vector<int>(s1);
        for (int t = 0; t <= this->num_jumps; t++) {
            index_parser_ns[i][t] = t * this->instance.num_vertices + i;
        }
    }

    for (int u = 0; u <= this->num_jumps; u++) {
        index_parser_s2[u] = vector<int>(s1);
        for (int t = 0; t <= this->num_jumps; t++) {
            index_parser_s2[u][t] = t * s1 + u;
        }
    }
    if (this->relaxed) {
        x = IloNumVarArray(env, ns, 0.0, 1.0, ILOFLOAT);
        r = IloNumVarArray(env, ns, 0.0, 1.0, ILOFLOAT);
        f = IloNumVarArray(env, ns, 0.0, 1.0, ILOFLOAT);
        g = IloNumVarArray(env, ns, 0.0, 1.0, ILOFLOAT);

        a = IloArray<IloNumVarArray>(env, m);
        for (int i = 0; i < m; i++) {
            a[i] = IloNumVarArray(env, s1, 0.0, 1.0, ILOFLOAT);
        }

        h = IloArray<IloNumVarArray>(env, m);
        for (int i = 0; i < m; i++) {
            h[i] = IloNumVarArray(env, s2, 0.0, 1.0, ILOFLOAT);
        }

        w = IloArray<IloNumVarArray>(env, ns);
        for (int i = 0; i < ns; i++) {
            w[i] = IloNumVarArray(env, s1, 0.0, 1.0, ILOFLOAT);
        }
    } else {
        x = IloNumVarArray(env, ns, 0.0, 1.0, ILOBOOL);
        r = IloNumVarArray(env, ns, 0.0, 1.0, ILOBOOL);
        f = IloNumVarArray(env, ns, 0.0, 1.0, ILOBOOL);
        g = IloNumVarArray(env, ns, 0.0, 1.0, ILOBOOL);

        a = IloArray<IloNumVarArray>(env, m);
        for (int i = 0; i < m; i++) {
            a[i] = IloNumVarArray(env, s1, 0.0, 1.0, ILOBOOL);
        }

        h = IloArray<IloNumVarArray>(env, m);
        for (int i = 0; i < m; i++) {
            h[i] = IloNumVarArray(env, s2, 0.0, 1.0, ILOBOOL);
        }

        w = IloArray<IloNumVarArray>(env, ns);
        for (int i = 0; i < ns; i++) {
            w[i] = IloNumVarArray(env, s1, 0.0, 1.0, ILOBOOL);
        }
    }

    for (int i = 0; i < this->instance.num_vertices; i++) {
        for (int j = 0; j < this->instance.num_vertices; j++) {
            for (int t = 0; t < s1; t++) {
                std::string name = "a_" + std::to_string(i) + "_" +
                                   std::to_string(j) + "_" + std::to_string(t);
                auto idx_ij = index_parser_m[i][j];
                a[idx_ij][t].setName(name.c_str());
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
                    h[idx_ij][idx_tu].setName(name.c_str());
                }
            }
        }
    }

    for (int i = 0; i < this->instance.num_vertices; i++) {
        for (int t = 0; t < s1; t++) {
            std::string name =
                "x_" + std::to_string(i) + "_" + std::to_string(t);
            auto idx = index_parser_ns[i][t];
            x[idx].setName(name.c_str());

            name = "r_" + std::to_string(i) + "_" + std::to_string(t);
            idx = index_parser_ns[i][t];
            r[idx].setName(name.c_str());

            name = "f_" + std::to_string(i) + "_" + std::to_string(t);
            idx = index_parser_ns[i][t];
            f[idx].setName(name.c_str());

            name = "g_" + std::to_string(i) + "_" + std::to_string(t);
            idx = index_parser_ns[i][t];
            g[idx].setName(name.c_str());

            for (int u = 0; u < s1; u++) {
                name = "w_" + std::to_string(i) + "_" + std::to_string(t) +
                       "_" + std::to_string(u);
                idx = index_parser_ns[i][t];
                w[idx][u].setName(name.c_str());
            }
        }
    }
}

void FeasibilityCharacterization::add_constraints() {
    // sum_{t \in {0..num_jumps}} x_{i,t} = 1 \forall i \in V (1)
    for (int i = 0; i < this->instance.num_vertices; i++) {
        IloExpr sum_x(env);
        for (int t = 0; t <= this->num_jumps; t++) {
            auto idx = index_parser_ns[i][t];
            sum_x += x[idx];
        }
        cplex_model.add(sum_x == 1);
    }

    // sum_{i \in V} x_{i,t} \geq 1 \forall t \in {0..num_jumps} (não tem no
    // latex)
    for (int t = 0; t <= this->num_jumps; t++) {
        IloExpr sum_x(env);
        for (int i = 0; i < this->instance.num_vertices; i++) {
            auto idx = index_parser_ns[i][t];
            sum_x += x[idx];
        }
        cplex_model.add(sum_x >= 1);
    }

    // x_{i,t} - a_{i,j,t} \geq 0 \forall (i,j) \in E^', \forall t \in
    // {0..num_jumps} (2)
    // x_{j,t} - a_{i,j,t} \geq 0 \forall (i,j) \in E^',
    // \forall t \in {0..num_jumps} (3)
    // a_{i,j,t} - x_{i,t} - x_{j,t} \geq -1
    // \forall (i,j) \in E^', \forall t \in {0..num_jumps} (4)
    // \sum_{u \in {0..num_jumps}, u>t} x_{iu} + x_{jt} \leq 1 & \forall (i,j)
    // \in R^-, \forall t \in {0..num_jumps} (5)
    for (auto const e : boost::make_iterator_range(
             boost::edges(this->instance.covering_graph))) {
        auto i = this->instance.covering_graph[e].source_id;
        auto j = this->instance.covering_graph[e].target_id;
        for (int t = 0; t <= this->num_jumps; t++) {
            auto idx_it = index_parser_ns[i][t];
            auto idx_jt = index_parser_ns[j][t];
            auto idx_m = index_parser_m[i][j];
            cplex_model.add(x[idx_it] >= a[idx_m][t]);
            cplex_model.add(x[idx_jt] >= a[idx_m][t]);
            cplex_model.add(a[idx_m][t] - x[idx_jt] - x[idx_it] >= -1);

            IloExpr sum_x(env);
            auto add = false;

            for (int u = t + 1; u <= this->num_jumps - 1; u++) {
                auto idx_iu = index_parser_ns[i][u];
                sum_x += x[idx_iu];
            }
            if (add) {
                sum_x += x[idx_jt];
                cplex_model.add(sum_x <= 1);
            }
        }
    }

    // r_{root,0} = 1 (8)
    cplex_model.add(r[index_parser_ns[this->instance.root][0]] == 1);

    // sum_{i \in V} r_{i, num_jumps} = 1 \forall t \in {0..num_jumps} (9)
    for (int t = 0; t <= this->num_jumps; t++) {
        IloExpr sum_r(env);
        for (int i = 0; i < this->instance.num_vertices; i++) {
            auto idx = index_parser_ns[i][t];
            sum_r += r[idx];
        }
        cplex_model.add(sum_r == 1);
    }

    //
    for (int i = 0; i < this->instance.num_vertices; i++) {
        for (int j = 0; j < this->instance.num_vertices; j++) {
            if (this->instance.exist_edge_in_covering_graph(i, j)) {
                for (int u = 0; u <= this->num_jumps; u++) {
                    IloExpr sum_x(env);
                    bool add = false;
                    for (int t = u + 1; t <= this->num_jumps; t++) {
                        sum_x += x[index_parser_ns[i][t]];
                        add = true;
                    }
                    if (add) {
                        sum_x += x[index_parser_ns[j][u]];
                        cplex_model.add(sum_x <= 1);
                    }
                }
            }
        }
    }

    // x_{jt} \geq r_{jt} & \forall j \in V/\{0\}, \forall t \in {0..num_jumps}
    // (6)
    for (int i = 0; i < this->instance.num_vertices; i++) {
        for (int t = 0; t <= this->num_jumps; t++) {
            auto idx = index_parser_ns[i][t];
            cplex_model.add(x[idx] >= r[idx]);
        }
    }

    // \sum_{t \in \pi} r_{jt} + \sum_{i \in \phi^-(j)}\sum_{t \in \pi}a_{ijt} =
    // 1 & \forall j \in V/\{0\} (7)
    for (int j = 0; j < this->instance.num_vertices; j++) {
        if (j != this->instance.root) {
            IloExpr sum_a(env);
            IloExpr sum_r(env);
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
            cplex_model.add(sum_r + sum_a == 1);
        }
    }

    // \sum_{t\in \pi} a_{ijt} \leq 1& \forall (i,j) \in R^- (10)
    for (int j = 0; j < this->instance.num_vertices; j++) {
        if (j != this->instance.root) {
            IloExpr sum_a(env);
            for (int t = 0; t <= this->num_jumps; t++) {
                for (int i : this->instance.covering_predecessors.at(j)) {
                    auto idx_m = index_parser_m[i][j];
                    sum_a += a[idx_m][t];
                }
            }
            cplex_model.add(sum_a <= 1);
        }
    }

    // não tem na implementação com ortools nem no pdf
    // for (int i = 0; i < this->instance.num_vertices; i++) {
    //     auto idx_it = index_parser_ns[i][0];
    //     cplex_model.add(f[idx_it] == 0);
    // }

    // \sum_{i \in V} f_{it} = 1 & \forall t > 0, t  \in {0..num_jumps} (12)
    for (int t = 1; t <= this->num_jumps; t++) {
        IloExpr sum_f(env);
        for (int i = 0; i < this->instance.num_vertices; i++) {
            auto idx_it = index_parser_ns[i][t];
            sum_f += f[idx_it];
        }
        cplex_model.add(sum_f == 1);
    }

    // f_{iu} \leq \sum_{t \in \pi, t < u} x_{it} & \forall u > 0, u \in
    // {0..num_jumps}, \forall i \in V (13)
    for (int u = 1; u <= this->num_jumps; u++) {
        for (int i = 0; i < this->instance.num_vertices; i++) {
            IloExpr sum_x(env);
            for (int t = 0; t < u; t++) {
                auto idx_it = index_parser_ns[i][t];
                sum_x += x[idx_it];
            }
            auto idx_iu = index_parser_ns[i][u];
            cplex_model.add(f[idx_iu] <= sum_x);
        }
    }

    // f_{it} + r_{jt} + d_{ij} \leq 2 & \forall i,j \in V, \forall t  \in
    // {1..num_jumps} (14)
    for (int i = 0; i < this->instance.num_vertices; i++) {
        for (int j = 0; j < this->instance.num_vertices; j++) {
            for (int t = 1; t <= this->num_jumps; t++) {
                auto idx_it = index_parser_ns[i][t];
                auto idx_jt = index_parser_ns[j][t];
                int d_ij = this->instance.exist_edge_in_order_graph(i, j);

                cplex_model.add(f[idx_it] + r[idx_jt] + d_ij <= 2);
            }
        }
    }

    // h_{ijtu} \leq f_{ju} & \forall i,j \in V, \forall u,t \in {0..num_jumps},
    // t < u (16)
    // h_{ijtu} \leq x_{jt} & \forall i,j \in V, \forall u,t \in
    // {0..num_jumps}, t < u (17)
    // h_{ijtu} \leq g_{it} & \forall i,j \in V,
    // \forall u,t \in {0..num_jumps}, t < u (18)
    // h_{ijtu} \geq f_{ju} + x_{jt}  + g_{it} - 2 & \forall i,j \in V,
    // \forall u,t \in {0..num_jumps}, t < u (19)
    for (int i = 0; i < this->instance.num_vertices; i++) {
        for (int j = 0; j < this->instance.num_vertices; j++) {
            for (int t = 0; t <= this->num_jumps; t++) {
                for (int u = t + 1; u <= this->num_jumps; u++) {
                    auto idx_ij = index_parser_m[i][j];
                    auto idx_tu = index_parser_s2[t][u];
                    auto idx_ju = index_parser_ns[j][u];
                    auto idx_jt = index_parser_ns[j][t];
                    auto idx_it = index_parser_ns[i][t];

                    cplex_model.add(h[idx_ij][idx_tu] <= f[idx_ju]);
                    cplex_model.add(h[idx_ij][idx_tu] <= x[idx_jt]);
                    cplex_model.add(h[idx_ij][idx_tu] <= g[idx_it]);
                    cplex_model.add(h[idx_ij][idx_tu] >=
                                    f[idx_ju] + x[idx_jt] + g[idx_it] - 2);
                }
            }
        }
    }

    // g_{iu} = f_{iu} + \sum_{j \in V} \sum_{t \in {0..num_jumps}, t < u}
    // h_{ijtu} & \forall i \in V, \forall u \in {1..num_jumps} (20)
    for (int i = 0; i < this->instance.num_vertices; i++) {
        // if (i != this->instance.root) {
        for (int u = 1; u <= this->num_jumps; u++) {
            IloExpr sum_h(env);
            for (int j = 0; j < this->instance.num_vertices; j++) {
                for (int t = 0; t < u; t++) {
                    auto idx_ij = index_parser_m[i][j];
                    auto idx_tu = index_parser_s2[t][u];
                    sum_h += h[idx_ij][idx_tu];
                }
            }
            auto idx_iu = index_parser_ns[i][u];
            cplex_model.add(g[idx_iu] == f[idx_iu] + sum_h);
        }
        //}
    }

    // w_{itu} \leq x_{it} & \forall i \in V, \forall u,t \in {0..num_jumps}, t
    // < u (22)
    // w_{itu} \leq g_{iu} & \forall i \in V, \forall u,t \in
    // {0..num_jumps}, t < u (23)
    // w_{itu} \geq x_{it} + g_{iu}  - 1 & \forall i
    // \in V, \forall u,t \in {0..num_jumps}, t < u (24)
    for (int i = 0; i < this->instance.num_vertices; i++) {
        for (int t = 0; t <= this->num_jumps; t++) {
            for (int u = 0; u <= this->num_jumps; u++) {
                if (t != u) {
                    auto idx_it = index_parser_ns[i][t];
                    auto idx_iu = index_parser_ns[i][u];
                    cplex_model.add(w[idx_it][u] <= x[idx_it]);
                    cplex_model.add(w[idx_it][u] <= g[idx_iu]);
                    cplex_model.add(w[idx_it][u] >= x[idx_it] + g[idx_iu] - 1);
                }
            }
        }
    }

    // x_{it} + x_{ju} \leq 1 + \sum_{k \in \sigma^-[i]} w_{ktu} & \forall (i,j)
    // \in R^-, \forall t, u \in {0..num_jumps}, t < u (25)
    for (auto const e : boost::make_iterator_range(
             boost::edges(this->instance.covering_graph))) {
        auto i = this->instance.covering_graph[e].source_id;
        auto j = this->instance.covering_graph[e].target_id;
        for (int t = 0; t <= this->num_jumps; t++) {
            for (int u = 0; u <= this->num_jumps; u++) {
                if (t != u) {
                    IloExpr sum_q(env);
                    auto idx_kt = index_parser_ns[i][t];
                    sum_q += w[idx_kt][u];
                    for (int k : this->instance.sucessors.at(i)) {
                        idx_kt = index_parser_ns[k][t];
                        sum_q += w[idx_kt][u];
                    }
                    auto idx_it = index_parser_ns[i][t];
                    auto idx_ju = index_parser_ns[j][u];
                    cplex_model.add(x[idx_it] + x[idx_ju] <= 1 + sum_q);
                }
            }
        }
    }

    // g_{i0} = 0 \forall i \in V
    for (int i = 0; i < this->instance.num_vertices; i++) {
        auto idx_i0 = index_parser_ns[i][0];
        cplex_model.add(g[idx_i0] == 0);
    }
    // as restrições abaixo não estão na implementação do ortools
    // for (int i = 0; i < this->instance.num_vertices; i++) {
    //     for (int t = 1; t <= this->num_jumps; t++) {
    //         auto idx_it = index_parser_ns[i][t];
    //         cplex_model.add(g[idx_it] <= 1);
    //     }
    // }

    // for (int t = 1; t <= this->num_jumps; t++) {
    //     IloExpr sum_x_l(env), sum_x_r(env);
    //     for (int i = 0; i < this->instance.num_vertices; i++) {
    //         auto idx_it = index_parser_ns[i][t];
    //         sum_x_l += x[idx_it];
    //     }

    //     for (int i = 0; i < this->instance.num_vertices; i++) {
    //         auto idx_it = index_parser_ns[i][t - 1];
    //         sum_x_r += x[idx_it];
    //     }
    //     cplex_model.add(sum_x_l - sum_x_r <= 0);
    // }
}

void FeasibilityCharacterization::add_objective_function() {
    // this->cplex_model.add(IloMinimize(env, 0));
}

void FeasibilityCharacterization::extract_solution() {
    // this->cplex_model.add(IloMinimize(env, 0));
}

int FeasibilityCharacterization::idx_ns(int i, int j) { return index_parser_ns[i][j]; }


void FeasibilityCharacterization::run() {
    for (int attempt = 0; attempt < this->instance.num_vertices; attempt++) {
        this->set_num_jumps(attempt);
        std::cout << "[INFO] Tentativa " << attempt << std::endl;
        Optimizer::run();
        if (this->solved)
            break;
    }
}

}  // namespace optimizer
