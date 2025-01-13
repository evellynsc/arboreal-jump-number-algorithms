/*
 * solver.cpp
 *
 *  Created on: 22 de fev de 2021
 *      Author: evellyn
 */

#include "solver.h"

// #include "multi_flow_model.h"

namespace solver {
Solver::~Solver() { cplex_solver.end(); }

Solver::Solver(solver_params& solver_config, mip::MIP& model) {
    this->config = solver_config;
    this->num_jumps = 0;
    cplex_solver = IloCplex(model.get_cplex_model());
    this->status = -1;
}

void Solver::save(std::string instance_id, std::string format) {
    auto model_file_name = instance_id;
    if (format == "MPS") {
        model_file_name.append(".mps");
    }
    this->cplex_solver.exportModel(model_file_name.c_str());
}

void Solver::setup_cplex() {
    // https://www.ibm.com/docs/ru/icos/20.1.0?topic=parameters-mip-dynamic-search-switch
    // cplex_solver.setParam(IloCplex::Param::Preprocessing::Presolve, CPX_ON);
    //	cplex_solver.setParam(IloCplex::Param::MIP::Strategy::HeuristicFreq,
    // CPX_ON);
    // cplex_solver.setParam(IloCplex::Param::MIP::Strategy::RINSHeur, CPX_ON);
    // cplex_solver.setParam(IloCplex::Param::MIP::Strategy::FPHeur, CPX_ON);
    // cplex_solver.setParam(IloCplex::Param::Preprocessing::Linear, 0);
    // //1 when running branch and cut
    // cplex_solver.setParam(IloCplex::Param::MIP::Strategy::Search,
    // CPX_MIPSEARCH_AUTO); //CPX_MIPSEARCH_AUTO
    //
    /*cplex_solver.setParam(IloCplex::Param::TimeLimit, config.time_limit);


    cplex_solver.setParam(IloCplex::Param::MIP::Strategy::Probe, 1);
    cplex_solver.setParam(IloCplex::Param::MIP::Limits::ProbeDetTime, 4000);*/
    // https://www.ibm.com/docs/en/icos/20.1.0?topic=performance-memory-emphasis-letting-optimizer-use-disk-storage
    cplex_solver.setParam(IloCplex::Param::Threads, 4);
    cplex_solver.setParam(IloCplex::Param::MIP::Limits::TreeMemory,
                          config.tree_memory);
    cplex_solver.setParam(IloCplex::Param::WorkMem, config.tree_memory);
    cplex_solver.setParam(IloCplex::Param::MIP::Strategy::File, 3);
    cplex_solver.setParam(IloCplex::Param::MIP::Display, 4);
    cplex_solver.setParam(IloCplex::Param::Emphasis::MIP,
                          CPX_MIPEMPHASIS_FEASIBILITY);
}

//
//

void Solver::solve(ajns::properties& p) {
    std::cout << "solver.cpp\n";
    solve();
    p.num_jumps = num_jumps;
}

vector<double> Solver::get_values_main_variables() {
    std::cout << "solver::get_values_main_variables\n";
    return vector<double>(0);
}

int Solver::get_status() { return status; }

void Solver::reset_model_by_kernel(std::vector<bool> kernel) {}

// Solver::Solver(std::string model_name, ajns::Instance& instance,
//                bool linear_relaxation, std::vector<bool> zero_variables) {
//     this->config = solver_params();
//     this->num_jumps = 0;
//     this->status = -1;
//     this->model = NULL;
//     if (model_name == "flow") {
//         this->model = new MultiFlowModel(instance, true);
//     }
//     this->cplex_solver = IloCplex(model->get_cplex_model());
// }

}  // namespace solver
