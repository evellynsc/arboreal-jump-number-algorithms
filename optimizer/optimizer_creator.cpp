//
// Created by evellyn on 17.11.2024
//
#include "optimizer_creator.h"

#include "optimizer/characterization.h"
#include "optimizer/exponential.h"
#include "optimizer/iterative.h"
#include "optimizer/multiflow.h"
#include "optimizer/optimizer.h"
#include "utils/const.h"

namespace optimizer {
Optimizer* OptimizerCreator::create(ajns::Instance& _instance,
                                    AlgorithmType _type, bool _relaxed,
                                    SolverParameters& _parameters) {
    Optimizer* optimizer_obj = nullptr;
    AlgorithmIds ALGO_ID;
    switch (_type) {
        case EXPONENTIAL:
            optimizer_obj = new Exponential(_instance, _relaxed);
            break;
        case BRANCH_AND_CUT:
            // optimizer_obj = new (_instance, _relaxed);
            break;
        case MULTIFLOW:
            std::cout << "[INFO] Criando formulação multifluxo" << std::endl;
            optimizer_obj = new MultiFlow(_instance, _relaxed, _parameters);
            break;
        case ITERATIVE:
            std::cout << "[INFO] Criando formulação iterativa" << std::endl;
            optimizer_obj = new Iterative(_instance, _relaxed, _parameters);
            break;
        case CHARACTERIZATION:
            optimizer_obj = new Characterization(_instance, _relaxed);
            break;
        default:
            std::cerr << "[ERRO] Não é possível instanciar o otimizador "
                      << ALGO_ID.enum_to_str[_type] << std::endl;
    }
    return optimizer_obj;
}
}  // namespace optimizer
