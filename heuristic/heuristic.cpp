#include "heuristic.h"

namespace ajns
{
    std::list<std::pair<my_graph::vertex, my_graph::vertex>>
        Heuristic::find_edges_to_remove(
            std::map<my_graph::vertex, std::list<my_graph::vertex>>& map_x2_z) {
        auto map_x2_z_edges =
            std::map<std::pair<my_graph::vertex, my_graph::vertex>,
            std::list<std::pair<my_graph::vertex, my_graph::vertex>>>();

        for (auto it : map_x2_z) {
            auto x2 = it.first;
            for (auto z : it.second) {
                auto removed_jumps =
                    std::list<std::pair<my_graph::vertex, my_graph::vertex>>();

                for (auto violation : open_violations) {
                    auto violator = violation.first;
                    auto violator_is_suc =
                        (boost::edge(z, violator, order_extension).second or
                            violator == z);

                    for (auto x : violation.second) {
                        auto x_is_pred =
                            (boost::edge(x, x2, order_extension).second or x == x2);
                        // std::cout << "Checking violation (" << x << ", " << violator
                        //     << ") with jump (" << x2 << ", " << z << ")"
                        //     << std::endl;
                        if (x_is_pred and violator_is_suc) {
                            removed_jumps.emplace_back(x, violator);
                            // std::cout << "Removing edge (" << x << ", " << violator
                            //     << ") due to added jump (" << x2 << ", " << z
                            //     << ")" << std::endl;
                        }
                    }
                }

                map_x2_z_edges[{x2, z}] = removed_jumps;
            }
        }

        auto selected_jump = map_x2_z_edges.begin()->first;
        auto max_removed = map_x2_z_edges.begin()->second.size();
        for (const auto& candidate : map_x2_z_edges) {
            if (candidate.second.size() > max_removed) {
                max_removed = candidate.second.size();
                selected_jump = candidate.first;
            }
        }

        curr_added_jump = selected_jump;
        added_jumps.push_back(selected_jump);
        return map_x2_z_edges[selected_jump];
    }

    void Heuristic::update_violations(
        std::list<std::pair<my_graph::vertex, my_graph::vertex>>& edges) {
        for (auto edge : edges) {
            open_violations[edge.second].remove(edge.first);
            boost::remove_edge(edge.first, edge.second, extension);
            if (open_violations[edge.second].size() <= 1) {
                update_num_pred_violators(edge.second);
                open_violations.erase(edge.second);
            }
        }
    }

    void Heuristic::update_num_pred_violators(my_graph::vertex central) {
        (void)central;
    }

    void Heuristic::update_extension(
        std::map<my_graph::vertex, std::list<my_graph::vertex>>& map_x2_z) {
        auto edges_to_remove = find_edges_to_remove(map_x2_z);
        update_violations(edges_to_remove);
        boost::add_edge(
            curr_added_jump.first, curr_added_jump.second,
            my_graph::edge_info(-1, curr_added_jump.first, curr_added_jump.second,
                my_graph::ARTIFICIAL),
            extension);
    }

    void Heuristic::update_order_extension() {
        auto jump = curr_added_jump;
        boost::add_edge(
            jump.first, jump.second,
            my_graph::edge_info(-1, jump.first, jump.second, my_graph::ARTIFICIAL),
            order_extension);
        boost::add_edge(
            jump.second, jump.first,
            my_graph::edge_info(-1, jump.second, jump.first, my_graph::ARTIFICIAL),
            t_order_extension);

        auto copy_order = my_graph::digraph();
        boost::copy_graph(order_extension, copy_order);
        auto copy_t_order = my_graph::digraph();
        boost::copy_graph(t_order_extension, copy_t_order);

        auto succ_tail = boost::adjacent_vertices(jump.second, copy_order);
        auto pred_head = boost::adjacent_vertices(jump.first, copy_t_order);

        for (auto u : boost::make_iterator_range(pred_head)) {
            for (auto v : boost::make_iterator_range(succ_tail)) {
                if (not boost::edge(u, v, order_extension).second) {
                    boost::add_edge(
                        u, v, my_graph::edge_info(-1, u, v, my_graph::ARTIFICIAL),
                        order_extension);
                    boost::add_edge(
                        v, u, my_graph::edge_info(-1, v, u, my_graph::ARTIFICIAL),
                        t_order_extension);
                }
            }
        }

        for (auto v : boost::make_iterator_range(succ_tail)) {
            if (not boost::edge(jump.first, v, order_extension).second) {
                boost::add_edge(
                    jump.first, v,
                    my_graph::edge_info(-1, jump.first, v, my_graph::ARTIFICIAL),
                    order_extension);
                boost::add_edge(
                    v, jump.first,
                    my_graph::edge_info(-1, v, jump.first, my_graph::ARTIFICIAL),
                    t_order_extension);
            }
        }

        for (auto v : boost::make_iterator_range(pred_head)) {
            if (not boost::edge(v, jump.second, order_extension).second) {
                boost::add_edge(
                    v, jump.second,
                    my_graph::edge_info(-1, jump.first, v, my_graph::ARTIFICIAL),
                    order_extension);
                boost::add_edge(
                    jump.second, v,
                    my_graph::edge_info(-1, v, jump.first, my_graph::ARTIFICIAL),
                    t_order_extension);
            }
        }
    }

    // Compute N^-_A[x1] \ N^-_A[x2] (predecessors of x1 not in predecessors of x2)
    std::list<my_graph::vertex> Heuristic::difference_x1_x2(
        my_graph::vertex x1, my_graph::vertex x2) {
        auto p_x1 = boost::adjacent_vertices(x1, t_order_extension);
        auto p_x2 = boost::adjacent_vertices(x2, t_order_extension);
        auto in_x2 = std::map<my_graph::vertex, bool>();
        auto x1_x2 = std::list<my_graph::vertex>();
        x1_x2.emplace_back(x1);
        in_x2[x1] = false;
        x1_x2.emplace_back(x2);
        in_x2[x2] = true;

        for (auto p : boost::make_iterator_range(p_x1)) {
            in_x2[p] = false;
            x1_x2.emplace_back(p);
        }
        for (auto p : boost::make_iterator_range(p_x2)) {
            in_x2[p] = true;
            x1_x2.emplace_back(p);
        }

        auto result = std::list<my_graph::vertex>();
        for (auto x : x1_x2) {
            if (not in_x2[x]) result.emplace_back(x);
        }
        return result;
    }
}