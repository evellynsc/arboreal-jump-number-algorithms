#include "solver_creator.hpp"

SolverCreator::SolverCreator() {}

SolverCreator::SolverCreator(ajns::Instance& instance,
                             solver::solver_params& config) {
    this->instance = instance;
}

void SolverCreator::create_model() { Solver* ptr = this->factory_method(); }

Solver* ExponentialCreator::factory_method() override {
    return new solver::BCSolver(solver_config, exp_model);
    ;
}