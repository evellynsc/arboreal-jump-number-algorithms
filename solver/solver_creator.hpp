#ifndef ALGORITHMCREATOR_HPP
#define ALGORITHMCREATOR_HPP

#include "base/instance.h"

namespace solver {
class AlgorithmCreator {
   public:
    SolverCreator();
    SolverCreator(ajns::Instance& instance, solver::solver_params&);
    virtual Solver* factory_method() = 0;
    void create_model();

   private:
    Solver* solver;
    MIP* model;
    ajns::Instance* instance;
};

class BranchCutCreator : public AlgorithmCreator {};

class ExponentialCreator : public AlgorithmCreator {};

}  // namespace solver

#endif  // ALGORITHMCREATOR_HPP
