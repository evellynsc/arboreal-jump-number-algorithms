/*
 * solution.cpp
 *
 *  Created on: 1 de mar de 2021
 *      Author: evellyn
 */

#include "solution.h"

Solution::Solution() { num_jumps = 0; }

Solution::Solution(int _njumps) { num_jumps = _njumps; }

void Solution::add_edge(int _id, int _i, int _j, bool is_jump) {
    my_graph::digraph::vertex_descriptor source, target;

    if (vertex_map.find(_i) == vertex_map.end()) {
        source = boost::add_vertex(my_graph::vertex_info(std::to_string(_i), _i),
            this->arboreal_extension);
        this->vertex_map[_i] = source;
    } else {
        source = this->vertex_map[_i];
    }
    if (vertex_map.find(_j) == vertex_map.end()) {
        target = boost::add_vertex(my_graph::vertex_info(std::to_string(_j), _j),
            this->arboreal_extension);
            this->vertex_map[_j] = target;
    } else {
        target = this->vertex_map[_j];
    }

    auto etype = is_jump ? my_graph::ARTIFICIAL : my_graph::ORIGINAL;
    boost::add_edge(source, target, my_graph::edge_info(_i, _j, etype), 
        this->arboreal_extension);
}

void Solution::save_to_file(std::string _file_name, std::string _file_extension) {
    auto dir = "dot/output/";
    std::ofstream file;
    file.open(dir + _file_name + "." + _file_extension);
    boost::write_graphviz(file, this->arboreal_extension, 
        boost::make_label_writer(boost::get(
            &my_graph::vertex_info::id, this->arboreal_extension)),
        boost::make_label_writer(boost::get(
        &my_graph::edge_info::type, this->arboreal_extension)));
    file.close();
}


