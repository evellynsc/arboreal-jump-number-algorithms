/*
 * model.cpp
 *
 *  Created on: Feb 22, 2022
 *      Author: evellyn
 */

#include "optimizer.h"

#include <ilcplex/ilocplex.h>
#include <vector>

#include <ostream>

#include "base/instance.h"
#include "utils/const.h"

namespace optimizer {

Optimizer::Optimizer() {
    std::cout << "[INFO] Iniciando resolvedor" << std::endl;
    this->env = IloEnv();
    this->cplex_model = IloModel(this->env);
    this->cplex_solver = IloCplex(this->env);
    this->solution = nullptr;
    this->solved = false;
    this->metrics = new Metrics();
    this->metrics->instance_name = this->instance.id;
    this->metrics->algorithm_name = AlgorithmIds().enum_to_str[this->type];
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
    this->save_model("lp");
}

void Optimizer::save_model(std::string _format) {
    auto algo_name = AlgorithmIds().enum_to_str[this->type];
    auto model_file_name = _format + "/" + this->instance.id + "-" + algo_name + "." + _format;
    std::cout << "[INFO] Salvando o modelo no arquivo " << model_file_name
              << std::endl;
    this->cplex_solver.exportModel(model_file_name.c_str());
}

Optimizer::~Optimizer() {
    this->save_model("mps");
    std::cout << "[INFO] Destruindo modelo" << std::endl;
    this->cplex_model.end();
    std::cout << "[INFO] Destruindo ambiente" << std::endl;
    this->env.end();
    std::cout << "[INFO] Destruindo solução" << std::endl;
    delete this->solution;
    std::cout << "[INFO] O modelo foi desalocado\n" << std::endl;
}

void Optimizer::print_metrics() {
    std::cout << "\n" << this->metrics->to_string() << std::endl;
}

void Optimizer::save_metrics(std::string directory) {
    auto metrics_file_name = directory + "metrics.csv";
    std::ofstream metrics_file;
    metrics_file.open(metrics_file_name, std::ios_base::app);
    if (metrics_file.is_open()) {
        metrics_file << this->metrics->to_string() << std::endl;
        metrics_file.close();
        std::cout << "[INFO] Métricas salvas em " << metrics_file_name << std::endl;
    } else {
        std::cerr << "[ERRO] Não foi possível abrir o arquivo de métricas "
                  << metrics_file_name << std::endl;
    }
}

void Optimizer::set_info(std::string instance_name, AlgorithmType type) {
    this->metrics->instance_name = instance_name;
    this->metrics->algorithm_name = AlgorithmIds().enum_to_str[type];
}

void Optimizer::run() {
    std::cout << "[INFO] Executando Optimizer::run()" << std::endl;
    std::cout << "[INFO] Construindo o modelo" << std::endl;
    build_model();
    std::cout << "[INFO] Configurando o resolvedor" << std::endl;
    setup();
    this->cplex_solver.setParam(IloCplex::Param::Preprocessing::Presolve, IloTrue);
    this->cplex_solver.setParam(IloCplex::Param::Preprocessing::Reduce, 3);
    // this->cplex_solver.setParam(IloCplex::Param::MIP::Limits::Nodes, 0);
    
    // this->cplex_solver.presolve();
    // exit(1);

    std::cout << "[INFO] Resolvendo o modelo" << std::endl;
    double start, finish;
    start = this->cplex_solver.getTime();
    this->solved = cplex_solver.solve();
    finish = this->cplex_solver.getTime();
    std::cout << "[INFO] Tempo de solução: " << finish - start << endl;
    this->metrics->solve_time = finish - start;
    if (this->solved) {
        this->cplex_solver.exportModel("reduced.lp");
        std::cout << "[INFO] Solução encontrada" << std::endl;
        auto status = this->cplex_solver.getStatus();
        this->metrics->num_explored_nodes = this->cplex_solver.getNnodes();
        this->metrics->num_jumps = this->cplex_solver.getObjValue();
        this->metrics->status = std::to_string(this->cplex_solver.getStatus());
        this->metrics->primal_bound = this->cplex_solver.getObjValue(); // For the best feasible solution found
        this->metrics->dual_bound = this->cplex_solver.getBestObjValue(); 
        this->metrics->cuts_added_by_solver = this->get_num_cuts();
        std::cout << "[INFO] Status da solução: " << status << std::endl;
        std::cout << "[INFO] Valor da solução: "
                  << this->metrics->num_jumps << std::endl;
        std::cout << "[INFO] Extraindo solução do modelo" << std::endl;
        extract_solution();
        if (this->solution != nullptr){
            std::cout << "[INFO] Salvando solução" << std::endl;
            this->solution->save_to_file(this->instance.id, "dot");
            std::cout << "[INFO] Solução salva com sucesso" << std::endl;
        }
        std::cout << "[INFO] Número total de cortes adicionados pelo cplex: " 
                  << this->get_num_cuts() << std::endl;
    } else {
        std::cerr << "[ERRO] O modelo é inviável."
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

int Optimizer::get_num_cuts() {
    std::vector<IloCplex::CutType> cut_types   
    {   IloCplex::CutType::CutCover      ,
        IloCplex::CutType::CutGubCover   ,
        IloCplex::CutType::CutFlowCover  ,
        IloCplex::CutType::CutClique     ,
        IloCplex::CutType::CutFrac       ,
        IloCplex::CutType::CutMir        ,
        IloCplex::CutType::CutFlowPath   ,
        IloCplex::CutType::CutDisj       ,
        IloCplex::CutType::CutImplBd     ,
        IloCplex::CutType::CutZeroHalf   ,
        IloCplex::CutType::CutMCF        ,
        IloCplex::CutType::CutLocalCover ,
        IloCplex::CutType::CutTighten    ,
        IloCplex::CutType::CutObjDisj    ,
        IloCplex::CutType::CutLiftProj   ,
        IloCplex::CutType::CutUser       ,
        IloCplex::CutType::CutTable      ,
        IloCplex::CutType::CutSolnPool   ,
        IloCplex::CutType::CutLocalImplBd,
        IloCplex::CutType::CutBQP        ,             
        IloCplex::CutType::CutRLT        ,             
        IloCplex::CutType::CutBenders    };                 
    int total_cuts = 0;
    for (auto cut_type : cut_types) {
        total_cuts += this->cplex_solver.getNcuts(cut_type);
    }
    return total_cuts;
}
}  // namespace optimizer
