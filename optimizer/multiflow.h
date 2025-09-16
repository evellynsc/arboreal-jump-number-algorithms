/*
 * multi_flow_model.h
 *
 *  Created on: Feb 23, 2022
 *      Author: evellyn
 */

#ifndef OPTIMIZER_MULTIFLOW_H_
#define OPTIMIZER_MULTIFLOW_H_

#include <boost/graph/transitive_closure.hpp>
#include <vector>
#include "optimizer.h"
#include "gurobi_c++.h"

namespace optimizer {
class MultiFlow : public Optimizer {
    /**
     * @brief
     * flow goes through arc (i,j) transporting token k
     * token k has to be delivered to vertex k
     * y(i,j) = 1 if arc (i,j) is selected to be in the solution, otherwise,
     * y(i,j) = 0
     */

    std::vector<GRBVar> var_f;
    std::vector<GRBVar> var_x;
    void add_variables() override;
    void add_objective_function() override;
    void add_constraints() override;
    std::vector<bool> zero_variables;
    void extract_solution() override;

   public:
    MultiFlow(ajns::Instance&);
    MultiFlow(ajns::Instance&, bool);
    MultiFlow(ajns::Instance&, bool, SolverParameters&);
    void reset_upper_bounds(std::vector<bool>);
    std::vector<GRBVar> get_y_variables();
    virtual ~MultiFlow() override = default;
    void run() override;
};

}  // namespace optimizer

#endif /* OPTIMIZER_MULTIFLOW_H_ */