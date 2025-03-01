#ifndef OPTIMIZER_FEASIBILITY_CHARACTERIZATION_H_
#define OPTIMIZER_FEASIBILITY_CHARACTERIZATION_H_

#include <ilcplex/ilocplex.h>

#include "base/instance.h"
#include "optimizer.h"

ILOSTLBEGIN

namespace optimizer {

class FeasibilityCharacterization : public Optimizer {
    void add_variables();
    void add_objective_function();
    void add_constraints();
    void extract_solution();

    IloNumVarArray x;
    IloNumVarArray r;
    IloNumVarArray f;
    IloNumVarArray g;

    IloArray<IloNumVarArray> a;
    IloArray<IloNumVarArray> h;
    IloArray<IloNumVarArray> w;

    vector<vector<int>> index_parser_ns;
    vector<vector<int>> index_parser_m;
    vector<vector<int>> index_parser_s2;

    int num_jumps;

   public:
    FeasibilityCharacterization(ajns::Instance&, bool);
    FeasibilityCharacterization(ajns::Instance&, bool, int);
    /*     IloNumVarArray get_v_variables();*/
    IloNumVarArray get_x_variables();
    IloNumVarArray get_r_variables();
    IloNumVarArray get_f_variables();
    IloNumVarArray get_g_variables();
    IloArray<IloNumVarArray> get_a_variables();
    int get_num_jumps();
    int idx_ns(int, int);
    void set_num_jumps(int);

    ~FeasibilityCharacterization() = default;
};
}  // namespace optimizer
#endif /* OPTIMIZER_CHARACTERIZATION_H_ */