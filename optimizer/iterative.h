/*
 * multi_flow_model.h
 *
 *  Created on: Feb 23, 2022
 *      Author: evellyn
 */

#ifndef OPTIMIZER_ITERATIVE_H_
#define OPTIMIZER_ITERATIVE_H_

#include <boost/graph/transitive_closure.hpp>
#include <vector>
#include "optimizer.h"
#include "gurobi_c++.h"

namespace optimizer {
class Iterative : public Optimizer {
    /**
     * @brief
     * flow goes through arc (i,j) transporting token k
     * token k has to be delivered to vertex k
     * y(i,j) = 1 if arc (i,j) is selected to be in the solution, otherwise,
     * y(i,j) = 0
     */

    std::vector<GRBVar> y;  // transitive closure (order graph)
    std::vector<GRBVar> x;  // transitive reduction (resulting graph)
    std::vector<GRBVar> u;  // auxiliary variable
    size_t t_max;
    void add_variables() override;
    void add_objective_function() override;
    void add_constraints() override;
    void extract_solution() override;

   public:
    Iterative(ajns::Instance&);
    Iterative(ajns::Instance&, bool);
    Iterative(ajns::Instance&, bool, SolverParameters&);
    void reset_upper_bounds(std::vector<bool>);
    std::vector<GRBVar> get_y_variables();
    virtual ~Iterative() override = default;
};

}  // namespace optimizer

#endif /* OPTIMIZER_ITERATIVE_H_ */