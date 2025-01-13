#ifndef UTILS_CONST_H_
#define UTILS_CONST_H_

#include <map>
#include <string>

enum AlgorithmType {
    EXPONENTIAL,
    BRANCH_AND_CUT,
    MULTIFLOW,
    ITERATIVE,  // talvez vá mudar de nome
    CHARACTERIZATION,
    RANDOM_HEURISTIC,
    SMART_HEURISTIC
};

struct AlgorithmIds {
    AlgorithmIds() = default;
    AlgorithmIds(const AlgorithmIds &) = default;
    AlgorithmIds(AlgorithmIds &&) = default;
    AlgorithmIds &operator=(const AlgorithmIds &) = default;
    AlgorithmIds &operator=(AlgorithmIds &&) = default;

    std::string CHARACTERIZATION_NAME = "CHARACTERIZATION";
    std::string MULTIFLOW_NAME = "MULTIFLOW";
    std::string ITERATIVE_NAME = "ITERATIVE";
    std::string BRANCH_AND_CUT_NAME = "BRANCH_AND_CUT";
    std::string EXPONENTIAL_NAME = "EXPONENTIAL";
    std::string RANDOM_HEURISTIC_NAME = "RANDOM_HEURISTIC";
    std::string SMART_HEURISTIC_NAME = "SMART_HEURISTIC";

    std::map<std::string, AlgorithmType> str_to_enum = {
        {EXPONENTIAL_NAME, EXPONENTIAL},
        {BRANCH_AND_CUT_NAME, BRANCH_AND_CUT},
        {MULTIFLOW_NAME, MULTIFLOW},
        {ITERATIVE_NAME, ITERATIVE},
        {CHARACTERIZATION_NAME, CHARACTERIZATION},
        {RANDOM_HEURISTIC_NAME, RANDOM_HEURISTIC},
        {SMART_HEURISTIC_NAME, SMART_HEURISTIC}};

    std::map<int, std::string> enum_to_str = {
        {EXPONENTIAL, EXPONENTIAL_NAME},
        {BRANCH_AND_CUT, BRANCH_AND_CUT_NAME},
        {MULTIFLOW, MULTIFLOW_NAME},
        {ITERATIVE, ITERATIVE_NAME},
        {CHARACTERIZATION, CHARACTERIZATION_NAME},
        {RANDOM_HEURISTIC, RANDOM_HEURISTIC_NAME},
        {SMART_HEURISTIC, SMART_HEURISTIC_NAME}};
};

#endif /* UTILS_CONST_H_ */