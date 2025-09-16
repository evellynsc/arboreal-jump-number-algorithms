#ifndef OPTIMIZER_CHARACTERIZATION_H_
#define OPTIMIZER_CHARACTERIZATION_H_

#include <vector>
#include "base/instance.h"
#include "optimizer.h"
#include "gurobi_c++.h"

namespace optimizer {

class Characterization : public Optimizer {
    void add_variables() override;
    void add_objective_function() override;
    void add_constraints() override;
    void extract_solution() override;

    std::vector<GRBVar> x;
    std::vector<GRBVar> r;
    std::vector<GRBVar> f;
    std::vector<GRBVar> g;

    std::vector<std::vector<GRBVar>> a;
    std::vector<std::vector<GRBVar>> h;
    std::vector<std::vector<GRBVar>> w;

    std::vector<std::vector<int>> index_parser_ns;
    std::vector<std::vector<int>> index_parser_m;
    std::vector<std::vector<int>> index_parser_s2;

    int num_jumps;

   public:
    Characterization(ajns::Instance&, bool);
    std::vector<GRBVar> get_x_variables();
    std::vector<GRBVar> get_r_variables();
    std::vector<GRBVar> get_f_variables();
    std::vector<GRBVar> get_g_variables();
    std::vector<std::vector<GRBVar>> get_a_variables();
    int get_num_jumps();
    int idx_ns(int, int);

    ~Characterization() override = default;
};
}  // namespace optimizer
#endif /* OPTIMIZER_CHARACTERIZATION_H_ */