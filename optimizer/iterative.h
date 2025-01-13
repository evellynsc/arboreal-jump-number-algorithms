/*
 * multi_flow_model.h
 *
 *  Created on: Feb 23, 2022
 *      Author: evellyn
 */

#ifndef OPTIMIZER_ITERATIVE_H_
#define OPTIMIZER_ITERATIVE_H_

#include <ilcplex/ilocplex.h>
ILOSTLBEGIN

#include <boost/graph/transitive_closure.hpp>

#include "optimizer.h"

namespace optimizer {
class Iterative : public Optimizer {
    /**
     * @brief
     * flow goes through arc (i,j) transporting token k
     * token k has to be delivery to vertex k
     * y(i,j) = 1 if arc (i,j) is selected to be in the solution, otherwise,
     * y(i,j) = 0
     */

    IloNumVarArray y;  // transitive closure (order graph)
    IloNumVarArray x;  // transitive reduction (resulting graph)
    IloNumVarArray u;  // auxiliar variable
    size_t t_max;
    void add_variables();
    void add_objective_function();
    void add_constraints();
    void extract_solution();

   public:
    Iterative(ajns::Instance&);
    Iterative(ajns::Instance&, bool);
    Iterative(ajns::Instance&, bool, SolverParameters&);
    void reset_upper_bounds(std::vector<bool>);
    IloNumVarArray get_y_variables();
    virtual ~Iterative() = default;
};

}  // namespace optimizer

#endif /* OPTIMIZER_ITERATIVE_H_ */
