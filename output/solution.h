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
#include "../base/elementary.h"

class Solution {
    int num_jumps;
    my_graph::digraph arboreal_extension;
    std::map<int, my_graph::digraph::vertex_descriptor> vertex_map;

   public:
    Solution();
    Solution(int);
    void add_edge(int, int, int, bool);
    virtual ~Solution() = default;
    void save_to_file(std::string, std::string);
};

#endif /* BASE_SOLUTION_H_ */
