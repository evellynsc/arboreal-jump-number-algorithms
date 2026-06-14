// In a new header file, e.g., gurobi_add_min_cuts_cb.h

#pragma once

#include "gurobi_c++.h"
#include <boost/graph/graphviz.hpp>
#include <boost/graph/transitive_closure.hpp>
#include <map>
#include <numeric>
#include <vector>

#include "../../algorithms/flow.h"
#include "../../base/instance.h"

class AddMinCutsCallback : public GRBCallback {
    ajns::Instance problem_instance;
    // GRBVar* x_vars; 
    GRBVar* x_vars;

    int num_vars;
    void add_cut_from_x_vars();
    
public:
    AddMinCutsCallback(ajns::Instance&, GRBVar*);

protected:
    void callback() override;
    my_graph::digraph construct_candidate_graph_from_x_vars(double*);
};
