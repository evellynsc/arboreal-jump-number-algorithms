/*
 * model.h
 *
 *  Created on: 19 de abr de 2021
 *      Author: evellyn
 */

#ifndef OPTIMIZER_EXPONENTIAL_H_
#define OPTIMIZER_EXPONENTIAL_H_

#include <ilcplex/ilocplex.h>

#include "../base/instance.h"
#include "optimizer.h"

ILOSTLBEGIN

namespace optimizer {
class Exponential : public Optimizer {
    IloBoolVarArray x;
    bool in_the_set(unsigned, std::size_t);
    void add_objective_function();
    void add_variables();
    void add_constraints();
    void extract_solution();
    void add_number_of_edges_constraints();
    void add_limit_indegree_constraints();
    void add_cutset_constraints();
    void add_out_edges_constraints();
    void fix_arcs_value();
    void add_bidirected_constraints();

   public:
    Exponential(ajns::Instance&);
    Exponential(ajns::Instance&, bool);
    IloBoolVarArray get_variables_x();
    auto get_variables(int);
    virtual ~Exponential() = default;
};

}  // namespace optimizer

#endif /* OPTIMIZER_EXPONENTIAL_H_ */
