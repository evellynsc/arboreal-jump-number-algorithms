/*
 * model.cpp
 *
 *  Created on: Feb 22, 2022
 *      Author: evellyn
 */

#include "optimizer.h"

#include <vector>
#include <ostream>
#include <fstream>
#include <iostream>
#include "base/instance.h"
#include "utils/const.h"
#include "utils/time.h"

namespace optimizer
{

    Optimizer::Optimizer() {
        this->env = nullptr;
        this->gurobi_model = nullptr;
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

    GRBEnv* Optimizer::get_gurobi_env() { return this->env; }

    ajns::Instance Optimizer::get_ajnp_instance() { return this->instance; }

    AlgorithmType Optimizer::get_type() { return this->type; }

    GRBModel* Optimizer::get_gurobi_model() { return this->gurobi_model; }

    void Optimizer::build_model() {
        this->env = new GRBEnv();
        this->env->set(GRB_IntParam_LogToConsole, this->parameters.verbosity);
        this->gurobi_model = new GRBModel(*this->env);
        // std::cout << "Building model..." << std::endl;
        add_variables();
        add_constraints();
        add_objective_function();
        this->save_model("lp");
        // std::cout << "Model built!" << std::endl;
    }

    void Optimizer::save_model(std::string _format) {
        auto algo_name = AlgorithmIds().enum_to_str[this->type];
        auto model_file_name = _format + "/" + this->instance.id + "-" + algo_name + "." + _format;
        this->gurobi_model->write(model_file_name);
    }

    Optimizer::~Optimizer() {
        this->save_model("mps");
        // delete this->gurobi_model;
        // delete this->env;
        // Check for nullptr before deleting to prevent double-free
        // if a derived class (like FeasibilityCharacterization) has already cleaned up.
        if (this->gurobi_model)
            delete this->gurobi_model;
        if (this->env)
            delete this->env;
        delete this->solution;
        delete this->metrics;
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
        }
        else {
            std::cerr << "[ERRO] Não foi possível abrir o arquivo de métricas "
                << metrics_file_name << std::endl;
        }
    }

    void Optimizer::set_info(std::string instance_name, AlgorithmType type) {
        this->metrics->instance_name = instance_name;
        this->metrics->algorithm_name = AlgorithmIds().enum_to_str[type];
    }

    void Optimizer::add_callbacks() {
    }

    void Optimizer::run() {
        Timestamp* ti = NewTimestamp(), * tf = NewTimestamp();
        Timer* timer = GetTimer();
        timer->Clock(ti);
        this->build_model();
        this->setup();
        this->add_callbacks();
        this->save_model("mps");
        std::cout << "Optimizing model..." << std::endl;
        this->gurobi_model->optimize();
        std::cout << "Model optimized!" << std::endl;
        timer->Clock(tf);

        this->metrics->solve_time = timer->ElapsedTime(ti, tf);

        int status = this->gurobi_model->get(GRB_IntAttr_Status);

        if (status == GRB_OPTIMAL || status == GRB_SUBOPTIMAL || status == GRB_TIME_LIMIT) {
            try {
                this->solved = true;
                this->gurobi_model->write("reduced.lp");
                this->metrics->num_explored_nodes = static_cast<int>(this->gurobi_model->get(GRB_DoubleAttr_NodeCount));
                this->metrics->num_jumps = this->gurobi_model->get(GRB_DoubleAttr_ObjVal);
                this->metrics->status = std::to_string(status);
                this->metrics->primal_bound = this->gurobi_model->get(GRB_DoubleAttr_ObjVal);
                this->metrics->dual_bound = this->gurobi_model->get(GRB_DoubleAttr_ObjBound);
                this->metrics->cuts_added_by_solver = this->get_num_cuts();

                extract_solution();
                if (this->solution != nullptr) {
                    this->solution->save_to_file(this->instance.id, "dot");
                }
            }
            catch (const GRBException& e) {
                this->solved = false;
                std::cerr << "[ERRO] Nenhuma solução encontrada. Gurobi error code " << e.getErrorCode() << ": " << e.getMessage() << std::endl;
            }
        }
        else {
            this->solved = false;
            std::cerr << "[ERRO] O modelo é inviável." << std::endl;
        }

        delete ti;
        delete tf;
        DeleteTimer();
    }

    void Optimizer::setup() {
        // std::cout << "Setting up the model..." << std::endl;
        // Turn off presolve
        this->gurobi_model->set(GRB_IntParam_Presolve, 0);
        this->gurobi_model->set(GRB_IntParam_Threads, this->parameters.num_threads);
        this->gurobi_model->set(GRB_DoubleParam_SoftMemLimit, this->parameters.memory_tree);
        this->gurobi_model->set(GRB_DoubleParam_TimeLimit, this->parameters.time_limit);
        this->gurobi_model->set(GRB_IntParam_OutputFlag, this->parameters.verbosity);
        this->gurobi_model->set(GRB_DoubleParam_Heuristics, 0.0);
        // std::cout << "Model setup!" << std::endl;
    }

    int Optimizer::get_num_cuts() {
        int total_cuts = 0;
        // try {
        //     total_cuts += static_cast<int>(this->gurobi_model->get(GRB_DoubleAttr_CutCount));
        // } catch (...) {
        //     // Attribute may not be available for all models
        // }
        return total_cuts;
    }

}  // namespace optimizer