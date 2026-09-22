/*
 * minimal_extension.h
 *
 *  Created on: 2 de mai de 2021
 *      Author: evellyn
 */

#ifndef HEURISTIC_SMART_MINIMAL_EXTENSION_H_
#define HEURISTIC_SMART_MINIMAL_EXTENSION_H_

#include <algorithm>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/copy.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <list>
#include <unordered_map>

#include "../base/instance.h"
#include "../base/properties.h"
#include "heuristic.h"


namespace ajns
{

    struct violator_visitor : boost::default_dfs_visitor {
        boost::shared_ptr<std::list<my_graph::vertex>> targets;
        violator_visitor() : targets(new std::list<my_graph::vertex>()) {}

        template <typename Vertex, typename Graph>
        void discover_vertex(Vertex v, Graph const& g) {
            if (boost::in_degree(v, g) > 1) {
                targets->emplace_back(v);
            }
            boost::default_dfs_visitor::discover_vertex(v, g);
        }
    };

    class SmartMinimalExtension : public Heuristic {
    private:
        std::unordered_map<my_graph::vertex, unsigned> vertex_levels;
        std::unordered_map<my_graph::vertex, unsigned> num_pred_violators;
        std::unordered_map<my_graph::vertex, unsigned> num_succ_violators;
        my_graph::digraph solution;
        my_graph::vertex get_minimal_element(std::vector<my_graph::vertex> vertices,
            my_graph::digraph& graph);
        void find_vertex_levels();
        void find_num_pred_violators();
        void find_num_succ_violators();
        my_graph::vertex choose_violator();
        my_graph::vertex choose_vertex_x1(my_graph::vertex c);
        my_graph::vertex choose_vertex_x2(my_graph::vertex c, my_graph::vertex x1);
        std::map<my_graph::vertex, std::list<my_graph::vertex>> find_x2_z(
            my_graph::vertex c, my_graph::vertex x1);
        void update_vertex_levels();
        void update_num_pred_violators(my_graph::vertex central) override;
        void update_num_pred_violators();
        void print_vertex_levels();
        void print_num_pred_violators();
        void print_open_violations();
        std::list<my_graph::vertex> get_min_z(std::list<my_graph::vertex>&);
        void find_violations();

    public:
        SmartMinimalExtension(Instance& problem_instance);
        virtual ~SmartMinimalExtension();
        void run() override;
        void run(properties& p) override;
        my_graph::digraph get_solution() override;
    };

} /* namespace ajns */

// https://stackoverflow.com/questions/50583130/how-does-distance-recorder-work-in-boost-graph-library
// dinamic properties boost

#endif /* HEURISTIC_SMART_MINIMAL_EXTENSION_H_ */
