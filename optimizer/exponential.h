/*
 * model.h
 *
 *  Created on: 19 de abr de 2021
 *      Author: evellyn
 */

#ifndef OPTIMIZER_EXPONENTIAL_H_
#define OPTIMIZER_EXPONENTIAL_H_

#include "../base/instance.h"
#include "optimizer.h"
#include "gurobi_c++.h"

namespace optimizer {
class Exponential : public Optimizer {
    std::vector<GRBVar> x;
    bool in_the_set(unsigned, std::size_t);
    void add_objective_function() override;
    void add_variables() override;
    void add_constraints() override;
    void extract_solution() override;
    void add_number_of_edges_constraints();
    void add_limit_indegree_constraints();
    void add_cutset_constraints();
    void add_out_edges_constraints();
    void fix_arcs_value();
    void add_bidirected_constraints();
    bool with_cutset_constraints;
    //TODO: implementar usercut para adicionar cortes e fazer o branch-cut

   protected:
    void add_callbacks() override;

    public:
    Exponential(ajns::Instance&);
    Exponential(ajns::Instance&, bool, bool, SolverParameters&);
    std::vector<GRBVar> get_variables_x();
    std::vector<GRBVar> get_variables(int);
    virtual ~Exponential() override = default;
    void run() override;
};

}  // namespace optimizer

#endif /* OPTIMIZER_EXPONENTIAL_H_ */