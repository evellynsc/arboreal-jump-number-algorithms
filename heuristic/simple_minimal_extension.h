/*
 * minimal_extension_no_choice.h
 *
 * Simplified minimal arboreal extension algorithm with no smart choice heuristics.
 * Implements: A := P, then iteratively add (x2, z) where z is ANY minimal element
 * of N^-_A[x1] \ N^-_A[x2] for ANY chosen violation (v, x1, x2).
 *
 *  Created on: 2026-06-14
 *      Author: evellyn
 */

#ifndef HEURISTIC_SIMPLE_MINIMAL_EXTENSION_H_
#define HEURISTIC_SIMPLE_MINIMAL_EXTENSION_H_

#include <algorithm>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/copy.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/transitive_reduction.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <list>
#include <unordered_map>

#include "../base/instance.h"
#include "../base/properties.h"
#include "heuristic.h"


namespace ajns
{

    class SimpleMinimalExtension : public Heuristic {
    private:
        std::unordered_map<my_graph::vertex, unsigned> vertex_levels;
        std::vector<my_graph::vertex> violators;
        my_graph::digraph solution;

        void find_violations();
        void find_vertex_levels();
        my_graph::vertex choose_violator();
        my_graph::vertex choose_vertex_x1(my_graph::vertex c);
        my_graph::vertex choose_vertex_x2(my_graph::vertex c, my_graph::vertex x1);
        std::list<my_graph::vertex> get_minimal_elements(
            std::list<my_graph::vertex>& candidates);
        my_graph::vertex choose_z(std::list<my_graph::vertex>& min_elements);
        void update_violations(my_graph::vertex x, my_graph::vertex violator);
        void remove_redundant_covering_edges();
        void update_vertex_levels();

    public:
        SimpleMinimalExtension(Instance& problem_instance);
        virtual ~SimpleMinimalExtension();
        void run() override;
        void run(properties& p) override;
        my_graph::digraph get_solution() override;
    };

} /* namespace ajns */

#endif /* HEURISTIC_SIMPLE_MINIMAL_EXTENSION_H_ */
