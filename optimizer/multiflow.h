/*
 * multi_flow_model.h
 *
 *  Created on: Feb 23, 2022
 *      Author: evellyn
 */

#ifndef OPTIMIZER_MULTIFLOW_H_
#define OPTIMIZER_MULTIFLOW_H_

#include <ilcplex/ilocplex.h>
ILOSTLBEGIN

#include <boost/graph/transitive_closure.hpp>

#include "optimizer.h"

namespace optimizer {
class MultiFlow : public Optimizer {
    /**
     * @brief
     * flow goes through arc (i,j) transporting token k
     * token k has to be delivery to vertex k
     * y(i,j) = 1 if arc (i,j) is selected to be in the solution, otherwise,
     * y(i,j) = 0
     */

    IloNumVarArray var_f;
    IloNumVarArray var_x;
    void add_variables();
    void add_objective_function();
    void add_constraints();
    std::vector<bool> zero_variables;
    void extract_solution();

   public:
    MultiFlow(ajns::Instance&);
    MultiFlow(ajns::Instance&, bool);
    MultiFlow(ajns::Instance&, bool, SolverParameters&);
    void reset_upper_bounds(std::vector<bool>);
    IloNumVarArray get_y_variables();
    virtual ~MultiFlow() = default;
};

}  // namespace optimizer

#endif /* OPTIMIZER_MULTIFLOW_H_ */
