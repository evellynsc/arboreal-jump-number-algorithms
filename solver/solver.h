/*
 * solver.h
 *
 *  Created on: 22 de fev de 2021
 *      Author: evellyn
 */

#ifndef SOLVER_SOLVER_H_
#define SOLVER_SOLVER_H_

#include <ilcplex/ilocplex.h>
ILOSTLBEGIN

#include "../base/properties.h"
#include "../base/solution.h"
#include "model/model.h"

namespace solver {

struct solver_params {
    double time_limit;
    double tree_memory;
    // std::set<user_cut> cuts;  // vai pra solution
    bool add_initial_solution;

    solver_params() {
        time_limit = 18000;
        tree_memory = 20000;
        add_initial_solution = false;
    }

    solver_params(double time_limit, double memory_tree)
        : time_limit(time_limit), tree_memory(memory_tree) {}

    solver_params(double time_limit, double memory_tree,
                  bool add_initial_solution)
        : time_limit(time_limit),
          tree_memory(memory_tree),
          add_initial_solution(add_initial_solution) {}
};

class Solver {
   protected:
    solver_params config;
    IloCplex cplex_solver;
    std::size_t num_jumps;  // mudar aqui pra double e levar isso pra solution
    Solution* solution;
    mip::MIP* model;

    int status;

   public:
    // Solver();
    // Solver(std::string, ajns::Instance&, bool, std::vector<bool>);
    Solver(solver_params&, mip::MIP&);
    Solver(solver_params&);
    /* 	solver(solver_params&, Model&); */
    void setup_cplex();
    void solve(ajns::properties& p);
    void solve(mip::MIP&);
    int get_status();
    void save(std::string, std::string);
    virtual void solve() = 0;

    virtual void reset_model_by_kernel(std::vector<bool>);
    virtual vector<double> get_values_main_variables();
    virtual ~Solver() = 0;
};

} /* namespace solver */

#endif /* SOLVER_SOLVER_H_ */
