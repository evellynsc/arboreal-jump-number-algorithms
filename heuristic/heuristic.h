/*
 * heuristic.h
 *
 * Abstract base class for minimal arboreal extension heuristics.
 *
 * Created on: 2026-06-14
 * Author: evellyn
 */

#ifndef HEURISTIC_HEURISTIC_H_
#define HEURISTIC_HEURISTIC_H_

#include <list>
#include <map>
#include <utility>
#include <vector>

#include <boost/graph/copy.hpp>

#include "../base/instance.h"
#include "../base/properties.h"

namespace ajns
{
    /**
     * Abstract base class for heuristic algorithms that compute minimal
     * arboreal extensions of a given problem instance.
     */
    class Heuristic {
    protected:
        Instance problem_instance;
        std::map<my_graph::vertex, std::list<my_graph::vertex>> open_violations;
        std::vector<std::pair<my_graph::vertex, my_graph::vertex>> added_jumps;
        std::pair<my_graph::vertex, my_graph::vertex> curr_added_jump;
        my_graph::digraph t_order_extension;
        my_graph::digraph order_extension;
        my_graph::digraph extension;

        std::list<std::pair<my_graph::vertex, my_graph::vertex>>
            find_edges_to_remove(
                std::map<my_graph::vertex, std::list<my_graph::vertex>>& map_x2_z);
        void update_violations(
            std::list<std::pair<my_graph::vertex, my_graph::vertex>>& edges);
        virtual void update_num_pred_violators(my_graph::vertex central);
        void update_extension(
            std::map<my_graph::vertex, std::list<my_graph::vertex>>& map_x2_z);
        void update_order_extension();
        std::list<my_graph::vertex> difference_x1_x2(
            my_graph::vertex x1, my_graph::vertex x2);

    public:
        /**
         * Constructor
         * @param problem_instance The problem instance to solve
         */
        Heuristic(Instance& problem_instance)
            : problem_instance(problem_instance) {
        }

        /**
         * Virtual destructor
         */
        virtual ~Heuristic() = default;

        /**
         * Run the heuristic algorithm
         */
        virtual void run() = 0;

        /**
         * Run the heuristic algorithm and populate properties
         * @param p Properties object to populate with statistics
         */
        virtual void run(properties& p) = 0;

        /**
         * Get the computed solution graph
         * @return The solution as a directed graph
         */
        virtual my_graph::digraph get_solution() = 0;
    };

} /* namespace ajns */

#endif /* HEURISTIC_HEURISTIC_H_ */
