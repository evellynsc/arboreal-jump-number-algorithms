/*
 * solution.h
 *
 *  Created on: 1 de mar de 2021
 *      Author: evellyn
 */

#ifndef BASE_SOLUTION_H_
#define BASE_SOLUTION_H_

#include <boost/graph/graphviz.hpp>

#include "../base/graph.h"

// my_graph::digraph construct_from_edges(my_graph::digraph &input_graph,
// std::vector<bool> &selected_edges);

struct selected_arc {
    int x;
    int y;
    bool artificial;
};

class Solution {
    int num_jumps;
    my_graph::digraph arboreal_extension;

   public:
    Solution();
    void convert_model_solution_to_graph();
    virtual ~Solution() = default;
};

#endif /* BASE_SOLUTION_H_ */
