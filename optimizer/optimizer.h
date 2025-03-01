/*
 * model.h
 *
 *  Created on: Feb 21, 2022
 *      Author: evellyn
 */

#ifndef OPTIMIZER_OPTIMIZER_H_
#define OPTIMIZER_OPTIMIZER_H_

#include "base/instance.h"
#include "output/solution.h"
#include "ilconcert/iloalg.h"
#include "ilcplex/ilocplexi.h"
#include "utils/const.h"
ILOSTLBEGIN

namespace optimizer {

struct SolverParameters {
    double time_limit;
    double memory_tree;
    int num_threads;
    bool add_initial_solution;

    SolverParameters()
        : time_limit(3600),
          memory_tree(10000),
          num_threads(4),
          add_initial_solution(false) {}

    SolverParameters(double _time_limit, double _memory_tree, int _num_threads,
                     bool _add_initial_solution)
        : time_limit(_time_limit),
          memory_tree(_memory_tree),
          num_threads(_num_threads),
          add_initial_solution(_add_initial_solution) {}

    SolverParameters(double _time_limit, double _memory_tree, int _num_threads)
        : time_limit(_time_limit),
          memory_tree(_memory_tree),
          num_threads(_num_threads),
          add_initial_solution(false) {}

    SolverParameters(double _time_limit, double _memory_tree)
        : time_limit(_time_limit),
          memory_tree(_memory_tree),
          num_threads(4),
          add_initial_solution(false) {}
};

class Optimizer {
   protected:
    bool relaxed;
    AlgorithmType type;
    IloEnv env;
    IloModel cplex_model;
    IloCplex cplex_solver;
    ajns::Instance instance;
    Solution solution;
    IloAlgorithm::Status status;
    SolverParameters parameters;

    virtual void add_variables() = 0;
    virtual void add_objective_function() = 0;
    virtual void add_constraints() = 0;
    virtual void extract_solution() = 0;
    virtual void build_model();

   public:
    Optimizer();
    Optimizer(ajns::Instance&, AlgorithmType, bool);
    Optimizer(ajns::Instance&, AlgorithmType, bool, SolverParameters&);
    virtual ~Optimizer();
    void run();
    IloModel get_cplex_model();
    IloEnv get_cplex_env();
    ajns::Instance get_ajnp_instance();
    AlgorithmType get_type();
    Solution get_solution();

   private:
    void save_model();
    void setup();
};
}  // namespace optimizer

#endif /* OPTIMIZER_OPTIMIZER_H_ */
