#include "heuristic_optimizer.h"
#include "base/properties.h"
#include "utils/time.h"


namespace optimizer
{

    HeuristicOptimizer::HeuristicOptimizer(ajns::Instance& instance, SolverParameters& parameters, bool smart)
        : Optimizer(instance, smart ? SMART_HEURISTIC : SIMPLE_HEURISTIC, false, parameters) {
    }

    void HeuristicOptimizer::run() {
        ajns::properties p;
        std::cout << "[INFO] Executando heurística " << this->type << std::endl;
        ajns::Heuristic* heuristic = nullptr;
        if (this->type == SMART_HEURISTIC)
            heuristic = new ajns::SmartMinimalExtension(this->instance);
        else
            heuristic = new ajns::SimpleMinimalExtension(this->instance);
        Timestamp* ti = NewTimestamp(), * tf = NewTimestamp();
        Timer* timer = GetTimer();
        timer->Clock(ti);
        heuristic->run(p);
        timer->Clock(tf);
        this->metrics->solve_time = timer->ElapsedTime(ti, tf);
        this->metrics->num_jumps = p.num_jumps;
        this->solved = true;
        delete heuristic;
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
