/*
 * solution.h
 *
 *  Created on: 1 de mar de 2021
 *      Author: evellyn
 */

#ifndef BASE_SOLUTION_H_
#define BASE_SOLUTION_H_

#include <boost/graph/graphviz.hpp>

#include "graph.h"

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
    // my_graph::digraph construct_graph(int, std::vector<selected_arc>&);

   public:
    Solution();
    // Solution(int, int, std::vector<selected_arc>&);
    // my_graph::digraph construct_from_edges(my_graph::digraph& input_graph,
    //                                        std::vector<bool>&
    //                                        selected_edges);
    // void save_solution(std::string, std::string);
    virtual ~Solution() = default;
};

#endif /* BASE_SOLUTION_H_ */
