#include "instance.h"

namespace ajns
{
    Instance::Instance(){
        id = "";
        num_vertices = 0;
        num_edges = 0;
        input_graph = my_graph::digraph();
        artificial_arcs_pair = std::vector<std::pair<int, int>>();
        root = -1;
    }

    Instance::Instance(std::string id, my_graph::vertex root,
        std::vector<std::pair<int, int>> artificial_arcs_pair,
        my_graph::digraph& order_graph,
        const my_graph::digraph& t_order_graph,
        const my_graph::digraph& covering_graph,
        const my_graph::digraph& input_graph, map_vertex_set predecessors,
        map_vertex_set sucessors): 
            id(id),
            root(root),
            num_vertices(boost::num_vertices(input_graph)),
            num_edges(boost::num_edges(input_graph)),
            artificial_arcs_pair(artificial_arcs_pair),
            order_graph(order_graph),
            t_order_graph(t_order_graph),
            covering_graph(covering_graph),
            input_graph(input_graph),
            predecessors(predecessors),
            sucessors(sucessors) {
        construct_adj_matrices();
    }

    Instance::Instance(std::string id, my_graph::vertex root,
        my_graph::digraph& order_graph,
        const my_graph::digraph& t_order_graph,
        const my_graph::digraph& covering_graph,
        const my_graph::digraph& input_graph, map_vertex_set predecessors,
        map_vertex_set sucessors):
            id(id),
            root(root),
            num_vertices(boost::num_vertices(input_graph)),
            num_edges(boost::num_edges(input_graph)),
            order_graph(order_graph),
            t_order_graph(t_order_graph),
            covering_graph(covering_graph),
            input_graph(input_graph),
            predecessors(predecessors),
            sucessors(sucessors) {
        construct_adj_matrices();    
    }

    Instance::Instance(std::string id, my_graph::vertex root,
        my_graph::digraph& order_graph,
        const my_graph::digraph& t_order_graph,
        const my_graph::digraph& covering_graph,
        const my_graph::digraph& input_graph, map_vertex_set predecessors,
        map_vertex_set sucessors,
        std::vector<std::pair<int, int>> artificial_arcs_pair): 
            id(id),
            root(root),
            num_vertices(boost::num_vertices(input_graph)),
            num_edges(boost::num_edges(input_graph)),
            order_graph(order_graph),
            t_order_graph(t_order_graph),
            covering_graph(covering_graph),
            input_graph(input_graph),
            predecessors(predecessors),
            sucessors(sucessors),
            artificial_arcs_pair(artificial_arcs_pair) {
        construct_adj_matrices();
    }

    Instance::    Instance(std::string id, my_graph::vertex root,
        my_graph::digraph& order_graph,
        const my_graph::digraph& t_order_graph,
        const my_graph::digraph& covering_graph,
        const my_graph::digraph& input_graph, map_vertex_set predecessors,
        map_vertex_set sucessors, map_vertex_set covering_predecessors,
        map_vertex_set covering_sucessors,
        std::vector<std::pair<int, int>> artificial_arcs_pair):
            id(id),
            root(root),
            num_vertices(boost::num_vertices(input_graph)),
            num_edges(boost::num_edges(input_graph)),
            order_graph(order_graph),
            t_order_graph(t_order_graph),
            covering_graph(covering_graph),
            input_graph(input_graph),
            predecessors(predecessors),
            sucessors(sucessors),
            covering_predecessors(covering_predecessors),
            covering_sucessors(covering_sucessors),
            artificial_arcs_pair(artificial_arcs_pair) {
        construct_adj_matrices();
    }

    bool Instance::exist_edge_in_input_graph(size_t from, size_t to) {
        return adj_matrix_input_graph[from][to];
    }

    bool Instance::exist_edge_in_order_graph(size_t from, size_t to) {
        return adj_matrix_order_graph[from][to];
    }

    bool Instance::exist_edge_in_covering_graph(size_t from, size_t to) {
        return adj_matrix_covering_graph[from][to];
    }

    void Instance::construct_adj_matrices() {
        auto n = num_vertices;

        auto get_row_of_neighboors = [n](const my_graph::digraph& graph,
                                         const my_graph::vertex from) {
            auto row = std::vector<bool>(n, false);
            for (const auto& e :
                 boost::make_iterator_range(boost::out_edges(from, graph))) {
                auto to = boost::target(e, graph);
                row[to] = true;
            }
            return row;
        };

        for (auto from :
             boost::make_iterator_range(boost::vertices(input_graph))) {
            auto row = get_row_of_neighboors(input_graph, from);
            adj_matrix_input_graph.push_back(row);
        }

        for (auto from :
             boost::make_iterator_range(boost::vertices(order_graph))) {
            auto row = get_row_of_neighboors(order_graph, from);
            adj_matrix_order_graph.push_back(row);
        }

        for (auto from :
             boost::make_iterator_range(boost::vertices(covering_graph))) {
            auto row = get_row_of_neighboors(covering_graph, from);
            adj_matrix_covering_graph.push_back(row);
        }
    }

    int Instance::get_max_indegree() {
        int max_indegree = 0;
        for (auto v : boost::make_iterator_range(boost::vertices(this->covering_graph))) {
            auto in_degree = boost::in_degree(v, this->covering_graph);
            if (in_degree > max_indegree) {
                max_indegree = in_degree;
            }
        }
        return max_indegree;
    }
}