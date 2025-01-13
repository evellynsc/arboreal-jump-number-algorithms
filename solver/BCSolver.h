/*
 * solver.h
 *
 *  Created on: 22 de fev de 2021
 *      Author: evellyn
 */

#ifndef SOLVER_BCSOLVER_H_
#define SOLVER_BCSOLVER_H_

#include <ilcplex/ilocplex.h>
ILOSTLBEGIN

#include "model/exponential_model.h"
#include "solver.h"

namespace solver {
class BCSolver : public Solver {
    mip::Exponential model;

   public:
    /* BCSolver() = default; */
    BCSolver(solver_params& config_, mip::Exponential& model_)
        : Solver(config_, &model_) {
        model = model_;
    }
    void solve();
    virtual ~BCSolver() = default;
};

} /* namespace solver */

#endif /* SOLVER_BCSOLVER_H_ */
