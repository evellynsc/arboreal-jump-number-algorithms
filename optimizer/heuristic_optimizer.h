#ifndef OPTIMIZER_HEURISTIC_OPTIMIZER_H_
#define OPTIMIZER_HEURISTIC_OPTIMIZER_H_

#include "optimizer.h"
#include "heuristic/minimal_extension.h"

namespace optimizer {

class HeuristicOptimizer : public Optimizer {
public:
    HeuristicOptimizer(ajns::Instance& instance, SolverParameters& parameters);
    virtual ~HeuristicOptimizer() = default;

    void run() override;

private:
    void build_model() override;
    void add_variables() override;
    void add_objective_function() override;
    void add_constraints() override;
    void extract_solution() override;
};

} // namespace optimizer

#endif // OPTIMIZER_HEURISTIC_OPTIMIZER_H_
