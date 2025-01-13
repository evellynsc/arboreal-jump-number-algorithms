#pragma once

#include <ilcplex/ilocplex.h>

#include "model/characterization_model.h"
#include "solver.h"

ILOSTLBEGIN

namespace solver {
class CharacterizationBasedSolver : public Solver {
    mip::CharacterizationModel model;

   public:
    CharacterizationBasedSolver();
    CharacterizationBasedSolver(solver_params& solver_config,
                                mip::CharacterizationModel& ajnp_model)
        : Solver(solver_config) {
        model = ajnp_model;
    }
    void solve();
    ~CharacterizationBasedSolver();
};
}  // namespace solver
