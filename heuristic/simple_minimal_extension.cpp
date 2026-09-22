/*
 * SimpleMinimalExtension.cpp
 *
 * Simplified implementation: arbitrary choices at each step.
 *
 *  Created on: 2026-06-14
 *      Author: evellyn
 */

#include "simple_minimal_extension.h"

#include <boost/graph/graphviz.hpp>
#include <random>

namespace ajns
{
    SimpleMinimalExtension::SimpleMinimalExtension(Instance& problem_instance)
        : Heuristic(problem_instance) {
        this->vertex_levels = std::unordered_map<my_graph::vertex, unsigned>();
        this->open_violations =
            std::map<my_graph::vertex, std::list<my_graph::vertex>>();
        this->added_jumps =
            std::vector<std::pair<my_graph::vertex, my_graph::vertex>>();
        this->violators = std::vector<my_graph::vertex>();
        // Initialize: A := P (extension := covering_graph)
        this->extension = my_graph::digraph();
        boost::copy_graph(problem_instance.covering_graph, extension);
        // std::cout << "Initial extension (covering graph):" << std::endl;
        // boost::write_graphviz(std::cout, extension,
        //     boost::make_label_writer(
        //         boost::get(&my_graph::vertex_info::label, extension)));
        this->order_extension = my_graph::digraph();
        boost::copy_graph(problem_instance.order_graph, order_extension);

        this->t_order_extension = my_graph::digraph();
        boost::copy_graph(problem_instance.t_order_graph, t_order_extension);

        // find_vertex_levels();
        find_violations();
    }

    void SimpleMinimalExtension::find_violations() {
        // Detect all vertices with in-degree > 1
        violators.clear();
        for (auto v : boost::make_iterator_range(
            boost::vertices(extension))) {
            if (boost::in_degree(v, extension) > 1) {
                auto in_edges = boost::in_edges(v, extension);
                auto sources = std::list<my_graph::vertex>();
                // std::cout << extension[v].label << ":\t";
                for (auto e : boost::make_iterator_range(in_edges)) {
                    auto source = boost::source(e, extension);
                    sources.push_back(source);
                    // std::cout << extension[source].label << "\t";
                }
                // std::cout << std::endl;
                open_violations.insert({ v, sources });
                this->violators.push_back(v);
            }
        }
    }

    // void SimpleMinimalExtension::find_vertex_levels() {
    //     auto sorted_vertices = std::list<my_graph::vertex>();
    //     boost::topological_sort(extension, std::front_inserter(sorted_vertices));

    //     for (auto i : sorted_vertices) {
    //         if (extension[i].is_root) {
    //             vertex_levels[i] = 0;
    //         }
    //         else {
    //             auto in_e = boost::in_edges(i, extension);
    //             auto max_level = 0u;
    //             for (auto e : boost::make_iterator_range(in_e)) {
    //                 auto head = boost::source(e, extension);
    //                 if (vertex_levels[head] > max_level)
    //                     max_level = vertex_levels[head];
    //             }
    //             vertex_levels[i] = max_level + 1;
    //         }
    //     }
    // }

    // Choose a random violator from the current violation map
    my_graph::vertex SimpleMinimalExtension::choose_violator() {
        static std::mt19937 generator(std::random_device{}());
        std::uniform_int_distribution<std::size_t> distribution(
            0, violators.size() - 1);
        auto selected_node = violators[distribution(generator)];
        return open_violations.find(selected_node)->first;
    }

    // Choose ANY predecessor (first one in the list)
    my_graph::vertex SimpleMinimalExtension::choose_vertex_x1(
        my_graph::vertex c) {
        return *open_violations[c].begin();
    }

    // Choose ANY OTHER predecessor (second one, or first if only two)
    my_graph::vertex SimpleMinimalExtension::choose_vertex_x2(
        my_graph::vertex c, my_graph::vertex x1) {
        auto& predecessors = open_violations[c];
        for (auto x : predecessors) {
            if (x != x1) {
                return x;
            }
        }
        // Should not reach here if there are at least 2 predecessors
        return x1;
    }

    // Find minimal elements in the list (those with minimum vertex_level)
    std::list<my_graph::vertex>
        SimpleMinimalExtension::get_minimal_elements(
            std::list<my_graph::vertex>& candidates) {
        if (candidates.empty()) return std::list<my_graph::vertex>();

        auto min_level = vertex_levels[candidates.front()];
        auto minimal = std::list<my_graph::vertex>();

        for (auto v : candidates) {
            auto level = vertex_levels[v];
            if (level < min_level) {
                min_level = level;
                minimal.clear();
                minimal.emplace_back(v);
            }
            else if (level == min_level) {
                minimal.emplace_back(v);
            }
        }
        return minimal;
    }

    // Choose ANY minimal element (first one)
    my_graph::vertex SimpleMinimalExtension::choose_z(
        std::list<my_graph::vertex>& min_elements) {
        return *min_elements.begin();
    }

    void SimpleMinimalExtension::update_violations(
        my_graph::vertex x2, my_graph::vertex violator) {
        // After adding edge (x2, z), update the violation map
        // The violator may have reduced in-degree or new covering edges may emerge

        // Remove violated edges from the violation list
        auto edge_exists = boost::edge(x2, violator, extension).second;
        if (edge_exists) {
            open_violations[violator].remove(x2);

            // If violator now has only 1 predecessor, it's no longer a violation
            if (open_violations[violator].size() <= 1) {
                open_violations.erase(violator);
            }
        }
    }

    void SimpleMinimalExtension::update_vertex_levels() {
        auto last_level_head = vertex_levels[curr_added_jump.first];
        auto last_level_tail = vertex_levels[curr_added_jump.second];
        auto succ_tail =
            boost::adjacent_vertices(curr_added_jump.second, order_extension);

        if (vertex_levels[curr_added_jump.second] < last_level_head + 1)
            vertex_levels[curr_added_jump.second] = last_level_head + 1;

        for (auto v : boost::make_iterator_range(succ_tail)) {
            auto dist = vertex_levels[v] - last_level_tail;
            if (vertex_levels[v] < vertex_levels[curr_added_jump.second] + dist)
                vertex_levels[v] = vertex_levels[curr_added_jump.second] + dist;
        }
    }

    void SimpleMinimalExtension::remove_redundant_covering_edges() {
        // After adding jumps, remove covering edges that are now implied by transitivity
        std::vector<std::pair<my_graph::vertex, my_graph::vertex>> edges_to_remove;

        for (const auto& e : boost::make_iterator_range(boost::edges(extension))) {
            auto u = boost::source(e, extension);
            auto v = boost::target(e, extension);

            // Skip self-loops and artificial edges for now
            if (u == v) continue;
            if (extension[e].type == my_graph::ARTIFICIAL) continue;

            // Check if there's an alternative path from u to v through artificial edges
            // If yes, this covering edge can be removed
            auto bfs_visitor = boost::default_bfs_visitor();
            auto color_map = std::vector<boost::default_color_type>(
                boost::num_vertices(extension), boost::white_color);

            // Try to find path from u to v using only artificial edges
            bool has_alternative_path = false;

            for (auto [edge, edge_end] = boost::out_edges(u, extension);
                edge != edge_end; ++edge) {
                auto next = boost::target(*edge, extension);
                if (next == v && extension[*edge].type == my_graph::ARTIFICIAL) {
                    // Found direct artificial edge u->v
                    has_alternative_path = true;
                    break;
                }
                if (extension[*edge].type == my_graph::ARTIFICIAL && next != v) {
                    // BFS from next using only artificial edges to reach v
                    for (auto [e2, e2_end] = boost::out_edges(next, extension);
                        e2 != e2_end; ++e2) {
                        if (extension[*e2].type == my_graph::ARTIFICIAL &&
                            boost::target(*e2, extension) == v) {
                            has_alternative_path = true;
                            break;
                        }
                    }
                }
            }

            if (has_alternative_path) {
                edges_to_remove.push_back({ u, v });
            }
        }

        for (const auto& [u, v] : edges_to_remove) {
            boost::remove_edge(u, v, extension);
        }
    }

    void SimpleMinimalExtension::run(properties& p) {
        p.num_nodes = boost::num_vertices(extension);
        p.num_arcs = boost::num_edges(extension);
        p.num_violators = open_violations.size();
        run();
        p.num_jumps = added_jumps.size();
    }

    void SimpleMinimalExtension::run() {
        while (this->open_violations.size() > 0) {
            auto v = choose_violator();
            auto x1 = choose_vertex_x1(v);
            auto x2 = choose_vertex_x2(v, x1);
            auto diff = difference_x1_x2(x1, x2);

            auto minimal_elements = get_minimal_elements(diff);

            if (minimal_elements.empty()) {
                std::cout << "  Error: No minimal elements found!" << std::endl;
                exit(1);
            }
            auto z = choose_z(minimal_elements);
            bool edge_already_exists = boost::edge(x2, z, extension).second;

            if (edge_already_exists) {
                std::cout << "  Error: Edge already exists!" << std::endl;
                exit(1);
            }

            auto candidate =
                std::map<my_graph::vertex, std::list<my_graph::vertex>>();
            candidate[x2].push_back(z);
            update_extension(candidate);
        }

        // std::cout << "Algorithm completed!" << std::endl;
        // std::cout << added_jumps.size() << " ======> jumps added" << std::endl;

        // Compute transitive reduction for output
        /*auto aux_graph = my_graph::digraph();
        auto g_to_tr = std::map<my_graph::vertex, my_graph::vertex>();
        boost::transitive_reduction(
            extension, aux_graph, boost::make_assoc_property_map(g_to_tr),
            boost::get(&my_graph::vertex_info::index, extension));

        auto tr_extension = my_graph::digraph();
        boost::copy_graph(extension, tr_extension);
        for (const auto& e : boost::make_iterator_range(boost::edges(extension))) {
            auto head = boost::source(e, extension);
            auto tail = boost::target(e, extension);
            if (not boost::edge(g_to_tr[head], g_to_tr[tail], aux_graph).second) {
                boost::remove_edge(head, tail, tr_extension);
            }
        }

        this->solution = tr_extension;*/
    }

    my_graph::digraph SimpleMinimalExtension::get_solution() {
        return this->solution;
    }

    SimpleMinimalExtension::~SimpleMinimalExtension() {}

} /* namespace ajns */
