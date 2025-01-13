//
// Created by evellyn on 17.11.2024
//
#ifndef OPTIMIZER_OPTIMIZER_CREATOR_H_
#define OPTIMIZER_OPTIMIZER_CREATOR_H_

#include "optimizer.h"

namespace optimizer {
class OptimizerCreator {
   public:
    static Optimizer* create(ajns::Instance& _instance, AlgorithmType _type,
                             bool _relaxed, SolverParameters& _parameters);
};
}  // namespace optimizer

#endif