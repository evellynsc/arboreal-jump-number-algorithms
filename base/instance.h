/*
 * instance.h
 *
 *  Created on: 14 de fev de 2021
 *      Author: evellyn
 */

#ifndef PROBLEM_INSTANCE_H_
#define PROBLEM_INSTANCE_H_

#include <string>
#include <vector>

#include "graph.h"

using map_vertex_set =
    std::unordered_map<my_graph::vertex, std::set<my_graph::vertex>>;

namespace ajns {
class Instance {
   public:
    std::string id;
    my_graph::vertex root;
    int num_vertices;
    int num_edges;

    std::vector<std::pair<int, int>> artificial_arcs_pair;
    std::vector<my_graph::vertex_info> vertex_properties;
    std::vector<my_graph::edge_info> edge_properties;
    my_graph::digraph order_graph;
    my_graph::digraph t_order_graph;
    my_graph::digraph covering_graph;
    my_graph::digraph input_graph;

    map_vertex_set sucessors;
    map_vertex_set predecessors;

    map_vertex_set covering_sucessors;
    map_vertex_set covering_predecessors;

    //	TODO: make a vector to represent edge ids

    //		size_t num_edges;
    //		std::vector<size_t> inner_vertices;

    Instance();

    Instance(std::string id, my_graph::vertex root,
             std::vector<std::pair<int, int>> artificial_arcs_pair,
             my_graph::digraph& order_graph,
             const my_graph::digraph& t_order_graph,
             const my_graph::digraph& covering_graph,
             const my_graph::digraph& input_graph, map_vertex_set predecessors,
             map_vertex_set sucessors);

    Instance(std::string id, my_graph::vertex root,
             my_graph::digraph& order_graph,
             const my_graph::digraph& t_order_graph,
             const my_graph::digraph& covering_graph,
             const my_graph::digraph& input_graph, map_vertex_set predecessors,
             map_vertex_set sucessors);

    Instance(std::string id, my_graph::vertex root,
             my_graph::digraph& order_graph,
             const my_graph::digraph& t_order_graph,
             const my_graph::digraph& covering_graph,
             const my_graph::digraph& input_graph, map_vertex_set predecessors,
             map_vertex_set sucessors,
             std::vector<std::pair<int, int>> artificial_arcs_pair);

    Instance(std::string id, my_graph::vertex root,
             my_graph::digraph& order_graph,
             const my_graph::digraph& t_order_graph,
             const my_graph::digraph& covering_graph,
             const my_graph::digraph& input_graph, map_vertex_set predecessors,
             map_vertex_set sucessors, map_vertex_set covering_predecessors,
             map_vertex_set covering_sucessors,
             std::vector<std::pair<int, int>> artificial_arcs_pair);

    bool exist_edge_in_input_graph(size_t from, size_t to);
    bool exist_edge_in_order_graph(size_t from, size_t to);
    bool exist_edge_in_covering_graph(size_t from, size_t to);
    int get_max_indegree();

   private:
    std::vector<std::vector<bool>> adj_matrix_input_graph;
    std::vector<std::vector<bool>> adj_matrix_order_graph;
    std::vector<std::vector<bool>> adj_matrix_covering_graph;

    void construct_adj_matrices();
};

}  // namespace ajns

#endif /* PROBLEM_INSTANCE_H_ */
