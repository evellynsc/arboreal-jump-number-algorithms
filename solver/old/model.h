/*
 * model.h
 *
 *  Created on: Feb 21, 2022
 *      Author: evellyn
 */

#ifndef MODEL_MODEL_H_
#define MODEL_MODEL_H_

#include <ilcplex/ilocplex.h>

#include "../base/instance.h"

ILOSTLBEGIN

namespace optimizer {
enum model_type {
    NOT_DEFINED,
    CUTSET_EXP,
    RELAXED_CUTSET,
    COMPACT,
    DDL,
    CHARACTERIZATION
};

class Model {
   protected:
    bool linear_relaxation;
    model_type type;
    IloEnv env;
    IloModel cplex_model;
    ajns::Instance instance;

    virtual void add_variables() = 0;
    virtual void add_objective_function() = 0;
    virtual void add_constraints() = 0;

   public:
    Model();
    virtual ~Model();

    void set_model_type(model_type);
    void set_env(IloEnv&);
    void set_cplex_model(IloModel&);
    void set_problem_instance(ajns::Instance&);
    IloModel get_cplex_model();
    IloEnv get_cplex_env();
    ajns::Instance get_ajnp_instance();
    model_type get_type();
    void create();
};
};  // namespace optimizer

#endif /* MODEL_MODEL_H_ */
