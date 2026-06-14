#include "heuristic_optimizer.h"
#include "base/properties.h"
#include "utils/time.h"


namespace optimizer
{

    HeuristicOptimizer::HeuristicOptimizer(ajns::Instance& instance, SolverParameters& parameters)
        : Optimizer(instance, SMART_HEURISTIC, false, parameters) {
    }

    void HeuristicOptimizer::run() {
        ajns::properties p;
        ajns::minimal_extension heuristic(this->instance);
        Timestamp* ti = NewTimestamp(), * tf = NewTimestamp();
        Timer* timer = GetTimer();
        timer->Clock(ti);
        heuristic.run(p);
        timer->Clock(tf);
        this->metrics->solve_time = timer->ElapsedTime(ti, tf);
        this->metrics->num_jumps = p.num_jumps;
        this->solved = true;
    }

    void HeuristicOptimizer::build_model() {
        // Not needed for the heuristic
    }

    void HeuristicOptimizer::add_variables() {
        // Not needed for the heuristic
    }

    void HeuristicOptimizer::add_objective_function() {
        // Not needed for the heuristic
    }

    void HeuristicOptimizer::add_constraints() {
        // Not needed for the heuristic
    }

    void HeuristicOptimizer::extract_solution() {
        // Not needed for the heuristic
    }

} // namespace optimizer
