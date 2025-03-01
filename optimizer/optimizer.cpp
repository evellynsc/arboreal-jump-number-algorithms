/*
 * model.cpp
 *
 *  Created on: Feb 22, 2022
 *      Author: evellyn
 */

#include "optimizer.h"

#include <ilcplex/ilocplex.h>

#include <ostream>

#include "base/instance.h"
#include "utils/const.h"

namespace optimizer {

Optimizer::Optimizer() {
    std::cout << "[INFO] Iniciando resolvedor" << std::endl;
    this->env = IloEnv();
    this->cplex_model = IloModel(this->env);
    this->cplex_solver = IloCplex(this->env);
}

Optimizer::Optimizer(ajns::Instance& _instance, AlgorithmType _type,
                     bool _relaxed)
    : Optimizer() {
    this->instance = _instance;
    this->type = _type;
    this->relaxed = _relaxed;
}

Optimizer::Optimizer(ajns::Instance& _instance, AlgorithmType _type,
                     bool _relaxed, SolverParameters& _parameters)
    : Optimizer() {
    this->instance = _instance;
    this->type = _type;
    this->relaxed = _relaxed;
    this->parameters = _parameters;
}

IloEnv Optimizer::get_cplex_env() { return this->env; }

ajns::Instance Optimizer::get_ajnp_instance() { return this->instance; }

AlgorithmType Optimizer::get_type() { return this->type; }

IloModel Optimizer::get_cplex_model() { return (this->cplex_model); }

void Optimizer::build_model() {
    std::cout << "[INFO] Adicionando variáveis" << std::endl;
    add_variables();
    std::cout << "[INFO] Adicionando restrições" << std::endl;
    add_constraints();
    std::cout << "[INFO] Adicionando função objetivo" << std::endl;
    add_objective_function();
    this->cplex_solver.extract(this->cplex_model);
    save_model();
}

void Optimizer::save_model() {
    auto model_file_name = this->instance.id + ".lp";
    std::cout << "[INFO] Salvando o modelo no arquivo " << model_file_name
              << std::endl;
    this->cplex_solver.exportModel(model_file_name.c_str());
}

Optimizer::~Optimizer() {
    std::cout << "[INFO] Desalocando o modelo" << std::endl;
    // verificar se faz sentido isso aqui
    this->env.end();
    this->cplex_model.end();
    std::cout << "[INFO] O modelo foi desalocado\n" << std::endl;
}

void Optimizer::run() {
    std::cout << "[INFO] Construindo o modelo" << std::endl;
    build_model();
    std::cout << "[INFO] Configurando o resolvedor" << std::endl;
    setup();
    std::cout << "[INFO] Resolvendo o modelo" << std::endl;
    double start, finish;
    start = this->cplex_solver.getTime();
    auto solved = cplex_solver.solve();
    finish = this->cplex_solver.getTime();
    std::cout << "[INFO] Tempo de solução: " << finish - start << endl;
    if (solved) {
        std::cout << "[INFO] Solução encontrada" << std::endl;
        std::cout << "[INFO] Status da solução: "
                  << this->cplex_solver.getStatus() << std::endl;
        std::cout << "[INFO] Valor da solução: "
                  << this->cplex_solver.getObjValue() << std::endl;
        extract_solution();
    } else {
        std::cerr << "[ERRO] O solver não encontrou uma solução para a "
                     "instância."
                  << std::endl;
    }
}

void Optimizer::setup() {
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
    cplex_solver.setParam(IloCplex::Param::Threads,
                          this->parameters.num_threads);
    cplex_solver.setParam(IloCplex::Param::MIP::Limits::TreeMemory,
                          this->parameters.memory_tree);
    cplex_solver.setParam(IloCplex::Param::MIP::Strategy::File, 3);
    cplex_solver.setParam(IloCplex::Param::MIP::Display, 4);
    // cplex_solver.setParam(IloCplex::Param::Emphasis::MIP,
    //                       CPX_MIPEMPHASIS_FEASIBILITY);
}

}  // namespace optimizer
