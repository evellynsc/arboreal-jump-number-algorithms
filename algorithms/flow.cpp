/*
 * flow.cpp
 *
 *  Created on: 14 de abr de 2021
 *      Author: evellyn
 */

#include "flow.h"
#include <iostream>


namespace ajns {

void flow::add_missing_reversed_edges() {
	auto num_vertices = boost::num_vertices(this->network);
	auto rev_edge_ids = num_vertices*num_vertices;
	my_graph::edge_itr eit, eit_end;
	auto graph_copy = my_graph::digraph();
	boost::copy_graph(this->network, graph_copy);

	auto reverse = my_graph::edge();
	auto reverse_exists = false;
	for (const auto& e : boost::make_iterator_range(boost::edges(this->network))) {
		auto source_v = boost::source(e, this->network);
		auto target_v = boost::target(e, this->network);
		boost::tie(reverse, reverse_exists) = boost::edge(target_v, source_v, this->network);
		if (not reverse_exists) {
			my_graph::edge_info rev_info(rev_edge_ids++, target_v, source_v, my_graph::FLOW);
			boost::tie(reverse, reverse_exists) = boost::add_edge(target_v, source_v,
					rev_info, this->network);
			if (not reverse_exists) exit(1);
			this->network[reverse].capacity = 0.0;
		}
		this->reversed_edge_of[e] = reverse;
		this->reversed_edge_of[reverse] = e;
	}

	// std::cout << "--- reversed_edge_of map ---" << std::endl;
    // for (const auto& pair : this->reversed_edge_of) {
    //     const auto& edge_from = pair.first;
    //     const auto& edge_to = pair.second;
    //     std::cout << "Edge " << edge_from << " (id: " << this->network[edge_from].id 
    //               << ") is reversed by " << edge_to << " (id: " << this->network[edge_to].id 
    //               << ")" << std::endl;
    // }
    // std::cout << "----------------------------" << std::endl;
	// boost::print_graph(this->network, boost::get(boost::vertex_index, this->network));
	// std::cout << "Graph has " << boost::num_vertices(this->network) << " vertices and "
	// 		<< boost::num_edges(this->network) << " edges." << std::endl;
}

void flow::fill_aux_maps() {
	auto reverse = my_graph::edge();
	auto reverse_exists = false;
	for (const auto& e : boost::make_iterator_range(boost::edges(network))) {
		auto source_vertex = boost::source(e, network);
		auto target_vertex = boost::target(e, network);
		//boost::tie(reverse, reverse_exists) = boost::edge(target_vertex, source_vertex, network);
		//if (not reverse_exists) exit(1);
		//reversed_edge_of[e] = reverse;
		res_capacity_map[e] = 0.0;
//		std::cout << network[e].capacity << " ";
	}
//	std::cout << std::endl;
}

flow::flow(my_graph::digraph& _network) {
	this->current_min_cut = std::list<my_graph::edge>();
	this->network = _network;
	this->current_max_flow_value = -1.0;
	add_missing_reversed_edges();
	
	// fill_aux_maps();
}

std::set<my_graph::vertex> flow::get_set_s() {
	auto set_s = std::set<my_graph::vertex>();

	for (const auto& e : current_min_cut) {
		auto head = boost::source(e, network);
		set_s.insert(head);
	}

	return set_s;
}

std::set<my_graph::vertex> flow::get_set_bar_s() {
	auto set_bar_s = std::set<my_graph::vertex>();

	for (const auto& e : current_min_cut) {
		auto tail = boost::target(e, network);
		set_bar_s.insert(tail);
	}

	return set_bar_s;
}

void flow::run(my_graph::vertex source, my_graph::vertex target, algo_flow algo) {
	if (algo == PUSREL) {
		std::cout << "Using Push-Relabel algorithm" << std::endl;
		auto props = capacity_map(get(&my_graph::edge_info::capacity, this->network))
				.residual_capacity_map(boost::make_assoc_property_map(this->res_capacity_map))
				.color_map(make_assoc_property_map(this->vertex_coloring))
				.reverse_edge_map(boost::make_assoc_property_map(this->reversed_edge_of))
				.vertex_index_map(get(boost::vertex_index, this->network));
		std::cout << "Using Push-Relabel algorithm" << std::endl;
		current_max_flow_value = boost::push_relabel_max_flow(this->network, source, target, props);
	} else {
		current_max_flow_value = boost::boykov_kolmogorov_max_flow(network,
				boost::get(&my_graph::edge_info::capacity,network),
				boost::make_assoc_property_map(res_capacity_map),
				boost::make_assoc_property_map(reversed_edge_of),
				boost::make_assoc_property_map(vertex_coloring),
				boost::get(boost::vertex_index,network),
				source, target);
	}
}

double flow::get_max_flow_value() {
	return this->current_max_flow_value;
}

std::list<my_graph::edge> flow::get_min_cut() {
	// std::cout << "Vertex coloring after max flow computation:" << std::endl;
	// for (const auto& v : this->vertex_coloring) {
	// 	if (v.second == boost::default_color_type::black_color)
	// 		std::cout << v.first << " :: " << " black" << std::endl;
	// 	else 
	// 		std::cout << v.first << " :: " << " not black" << std::endl;
	// }
	// if (not current_min_cut.empty())
	// 	return current_min_cut;
	

	for (auto e : boost::make_iterator_range(boost::edges(this->network))) {
		/*if (network[e].type == my_graph::FLOW or network[e].type == my_graph::USINK) {
			current_min_cut.clear();
			break;
		}*/

		if (this->network[e].type == my_graph::ARTIFICIAL or this->network[e].type == my_graph::ORIGINAL) {
			auto head = boost::source(e, this->network);
			auto tail = boost::target(e, this->network);
			if (vertex_coloring[head] == boost::default_color_type::black_color and 
				vertex_coloring[tail] != boost::default_color_type::black_color) {
				this->current_min_cut.emplace_back(e);
			}
		}
	}
	//std::cout << std::endl;
	return this->current_min_cut;
}

flow::~flow() {
	// TODO Auto-generated destructor stub
}

} /* namespace ajns */
