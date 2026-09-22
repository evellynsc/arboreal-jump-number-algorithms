#include <vector>
#include "../base/graph.h"
// #include <boost/detail/workaround.hpp>

#include <boost/graph/detail/adjacency_list.hpp>


std::map<my_graph::vertex, std::list<my_graph::vertex>> find_violations(my_graph::digraph extension) {
    // Detect all vertices with in-degree > 1
    std::vector<my_graph::vertex> violators;
    std::map<my_graph::vertex, std::list<my_graph::vertex>> open_violations;
    for (auto v : boost::make_iterator_range(
        boost::vertices(extension))) {
        if (boost::in_degree(v, extension) > 1) {
            auto in_edges = boost::in_edges(v, extension);
            auto sources = std::list<my_graph::vertex>();
            std::cout << extension[v].label << ":\t";
            for (auto e : boost::make_iterator_range(in_edges)) {
                auto source = boost::source(e, extension);
                sources.push_back(source);
                std::cout << extension[source].label << "\t";
            }
            std::cout << std::endl;
            open_violations.insert({ v, sources });
            violators.push_back(v);
        }
    }
    return open_violations;
}


std::list<my_graph::vertex> difference_x1_x2(my_graph::vertex x1,
    my_graph::vertex x2, my_graph::digraph t_order_extension) {
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

std::list<std::pair<my_graph::vertex, my_graph::vertex>> find_edges_to_remove(
    std::map<my_graph::vertex, std::list<my_graph::vertex>>& map_x2_minz, std::map<my_graph::vertex, std::list<my_graph::vertex>>& open_violations, my_graph::digraph& order_extension) {
    auto removed_edges =
        std::list<std::pair<my_graph::vertex, my_graph::vertex>>();
    auto map_x2_z =
        std::map<std::pair<my_graph::vertex, my_graph::vertex>,
        std::list<std::pair<my_graph::vertex, my_graph::vertex>>>();
    auto vec_num_edges = std::vector<int>();

    for (auto it : map_x2_minz) {
        auto x2 = it.first;
        //		std::cout << "*x2 = " << x2 << "\t";
        auto array_z = it.second;
        for (auto z : array_z) {
            //			std::cout << "*z = " << z << "\t";
            auto cur_removed_edges =
                std::list<std::pair<my_graph::vertex, my_graph::vertex>>();
            auto removed_jumps =
                std::list<std::pair<my_graph::vertex, my_graph::vertex>>();
            for (auto j : open_violations) {
                auto violator = j.first;
                auto violator_is_suc =
                    (boost::edge(z, violator, order_extension).second or
                        violator == z);
                //				std::cout << violator_is_suc << " ((" << z <<
                //"," << violator << "))\t";
                auto array_x = j.second;
                for (auto x : array_x) {
                    auto x_is_pred =
                        (boost::edge(x, x2, order_extension).second or x == x2);
                    if (x_is_pred and violator_is_suc) {
                        removed_jumps.emplace_back(x, violator);
                    }
                }
            }

            map_x2_z[{x2, z}] = removed_jumps;
        }
    }
    auto max = map_x2_z.begin()->second.size();
    auto selected_jump = map_x2_z.begin()->first;
    for (auto i : map_x2_z) {
        auto jump = i.first;
        auto edges = i.second;
        if (edges.size() > max) {
            max = edges.size();
            selected_jump = jump;
        }
    }
    added_jumps.push_back(selected_jump);
    curr_added_jump = selected_jump;
    return map_x2_z[selected_jump];
}

void update_violations(
    std::list<std::pair<my_graph::vertex, my_graph::vertex>>& edges) {
    //	std::cout << "removing edges\n";
    for (auto e : edges) {
        this->open_violations[e.second].remove(e.first);
        boost::remove_edge(e.first, e.second, extension);
        if (this->open_violations[e.second].size() <= 1) {
            update_num_pred_violators(e.second);
            this->open_violations.erase(e.second);
        }
    }
}


void update_extension(
    std::map<my_graph::vertex, std::list<my_graph::vertex>>& map_x2_z, std::map<my_graph::vertex, std::list<my_graph::vertex>>& open_violations, my_graph::digraph& order_extension) {
    auto edges_to_remove = find_edges_to_remove(map_x2_z, open_violations, order_extension);
    update_violations(edges_to_remove);
    boost::add_edge(
        curr_added_jump.first, curr_added_jump.second,
        my_graph::edge_info(-1, curr_added_jump.first, curr_added_jump.second,
            my_graph::ARTIFICIAL),
        extension);
}



