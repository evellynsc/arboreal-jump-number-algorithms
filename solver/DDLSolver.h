#ifndef SOLVER_DDLSOLVER_H_
#define SOLVER_DDLSOLVER_H_

#pragma once

#include <ilcplex/ilocplex.h>

#include "ddl_model.h"
#include "solver.h"

ILOSTLBEGIN

namespace solver {
class DDLSolver : public Solver {
    DDLModel model;

   public:
    //	MFSolver();
    DDLSolver(solver_params& solver_config, DDLModel& ajnp_model)
        : Solver(solver_config, &ajnp_model) {
        model = ajnp_model;
    }
    void solve();
    virtual ~DDLSolver() = default;
};
} /* namespace solver */

#endif /* SOLVER_DDLSOLVER_H_ */