#pragma once

#include "../base/instance.h"
#include "MFSolver.h"
#include "solver.h"

class Kernel {
   private:
    int nbuck;
    solver::Solver* cpx;
    ajns::Instance* ajnp;

   public:
    Kernel();
    Kernel(ajns::Instance& ajnp, std::string solver_name, int nbuck);
    ~Kernel();
    void run();
};
