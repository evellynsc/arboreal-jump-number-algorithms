#include "add_min_cuts.h"
#include "algorithms/flow.h"
#include <boost/graph/graphviz.hpp>
#include <boost/graph/graph_utility.hpp> // For print_graph
#include <fstream>

AddMinCutsCallback::AddMinCutsCallback(ajns::Instance& _instance, GRBVar* _vars)
    : problem_instance(_instance), num_vars(_instance.num_edges), x_vars(_vars) {
}

my_graph::digraph AddMinCutsCallback::construct_candidate_graph_from_x_vars(double* x) {
    my_graph::digraph graph_x;
    boost::copy_graph(this->problem_instance.input_graph, graph_x);
    auto edges_to_remove = std::vector<my_graph::edge>();
    for (const auto& e : boost::make_iterator_range(boost::edges(graph_x))) {
        int var_idx = graph_x[e].id;
        if (var_idx < this->num_vars && x[var_idx] <= 1e-6) {
            edges_to_remove.push_back(e);
        }
    }
    for (const auto& e : edges_to_remove) {
        boost::remove_edge(e, graph_x);
    }
    return graph_x;
}

void AddMinCutsCallback::add_cut_from_x_vars() {
    double* x_values;
    if (where == GRB_CB_MIPSOL) {
        int nodecnt = (int)getDoubleInfo(GRB_CB_MIPSOL_NODCNT);
        double obj = getDoubleInfo(GRB_CB_MIPSOL_OBJ);
        int solcnt = getIntInfo(GRB_CB_MIPSOL_SOLCNT);
        x_values = getSolution(this->x_vars, this->num_vars);
    }
    else if (where == GRB_CB_MIPNODE) {
        x_values = getNodeRel(this->x_vars, this->num_vars);
    }
    else
        return;
    // std::cout << "**** New solution at node " << nodecnt
        // << ", obj " << obj << ", sol " << solcnt << std::endl;
    // for (int j = 0; j < this->num_vars; j++) {
    //     // if (x_values[j] > 1e-6) {
    //         // std::cout << "x[" << j << "] = " << x_values[j] << std::endl;
    //     // }
    // }
    auto graph_x = construct_candidate_graph_from_x_vars(x_values);
    // // std::cout << "Original Adjacency List:" << std::endl;
    // boost::print_graph(graph_x, boost::get(boost::vertex_index, graph_x));
    // // std::cout << std::endl;

    auto tc_graph_x = my_graph::digraph();

    boost::transitive_closure(graph_x, tc_graph_x);

    // boost::print_graph(problem_instance.covering_graph, boost::get(boost::vertex_index, problem_instance.covering_graph));
    // // std::cout << std::endl;

    // // std::cout << std::endl << boost::num_vertices(tc_graph_x) << " vertices and "
    //           << boost::num_edges(tc_graph_x) << " edges in transitive closure." << std::endl;
    // // std::cout << "Transitive Closure Adjacency List:" << std::endl;

    // boost::print_graph(tc_graph_x, boost::get(boost::vertex_index, tc_graph_x));

    // Print tc_graph_x to a .dot file
    // {
    //     std::ofstream out_file("tc_graph_x.dot");
    //     boost::write_graphviz(out_file, tc_graph_x,
    //         boost::make_label_writer(boost::get(&my_graph::vertex_info::id, tc_graph_x)),
    //         boost::make_label_writer(boost::get(&my_graph::edge_info::id, tc_graph_x)));
    // }

    auto complete_net_flow = my_graph::digraph();
    boost::copy_graph(problem_instance.input_graph, complete_net_flow);
    for (const auto& e :
        boost::make_iterator_range(boost::edges(complete_net_flow))) {
        int var_idx = complete_net_flow[e].id;
        complete_net_flow[e].capacity = x_values[var_idx];
        if (complete_net_flow[e].capacity <= 1e-6) {
            complete_net_flow[e].capacity = 0;
        }
    }
    // std::cout << "Complete net flow graph has "
            //   << boost::num_vertices(complete_net_flow) << " vertices and "
            //   << boost::num_edges(complete_net_flow) << " edges." << std::endl;
    for (const auto& e : boost::make_iterator_range(
        boost::edges(problem_instance.covering_graph))) {
        auto head = boost::source(e, problem_instance.covering_graph);
        auto tail = boost::target(e, problem_instance.covering_graph);
        // std::cout << "Checking edge (" << head << ", " << tail << ")\n";
        if (not boost::edge(head, tail, tc_graph_x).second
            and
            problem_instance.predecessors.at(head).size() != 0u and
            problem_instance.predecessors.at(tail).size() != 0u
            ) {
            // std::cout << "entrou aqui" << std::endl;
            auto set_q = std::set<my_graph::vertex>();

            set_q.insert(problem_instance.predecessors.at(head).begin(),
                problem_instance.predecessors.at(head).end());
            set_q.insert(problem_instance.sucessors.at(tail).begin(),
                problem_instance.sucessors.at(tail).end());

            my_graph::digraph net_flow;
            boost::copy_graph(complete_net_flow, net_flow);

            for (const auto& v : set_q) {
                boost::clear_vertex(v, net_flow);
            }

            auto flow_algo = ajns::flow(net_flow);
            flow_algo.run(head, tail, ajns::BOYKOL);
            // std::cout << "Max flow value: " << flow_algo.get_max_flow_value() << std::endl;

            if (flow_algo.get_max_flow_value() < 1.0 - 1e-6) {
                auto min_cut = flow_algo.get_min_cut();

                if (!min_cut.empty()) {
                    GRBLinExpr lhside_ineq = 0;
                    for (const auto& cut_edge : min_cut) {
                        int var_idx = problem_instance.input_graph[cut_edge].id;
                        lhside_ineq += this->x_vars[var_idx];
                        // std::cout << this->num_vars << " Cut edge: " << cut_edge << " with var x[" << var_idx << "]\n";
                    }
                    // std::cout << "Adding cut for edge (" << head << ", " << tail << ")\n";
                    if (where == GRB_CB_MIPSOL)
                        addLazy(lhside_ineq >= 1);
                    else if (where == GRB_CB_MIPNODE)
                        // todo: tá errado isso aqui, precisa conferir o status
                        addCut(lhside_ineq >= 1);
                    // addLazy(lhside_ineq >= 1);
                }
                else {
                    // std::cout << "Min cut is empty!" << std::endl;
                }
            }
        }
    }
    delete[] x_values;
}


void AddMinCutsCallback::callback() {
    try {
        this->add_cut_from_x_vars();
    }
    catch (GRBException e) {
        std::cout << "Error number: " << e.getErrorCode() << std::endl;
        std::cout << e.getMessage() << std::endl;
    }
    catch (...) {
        std::cout << "Error during callback" << std::endl;
    }

    // We are only interested in MIP node relaxations for generating user cuts.
    // if (where != GRB_CB_MIPNODE) {
    //     return;
    // }

    // // Check if the node LP was solved to optimality.
    // int optimstatus = getIntInfo(GRB_CB_MIPNODE_STATUS);
    // if (optimstatus != GRB_OPTIMAL) {
    //     return;
    // }

    // // std::cout << "Getting variable values..." << std::endl;
    // // Use getNodeRel to get the fractional solution of the LP relaxation.

    // if (where == GRB_CB_POLLING) {
    //     // std::cout << "pooling" << std::endl;
    // }
    // else if (where == GRB_CB_MIPSOL) {

    //     delete[] x_values;
    // } else if (where == GRB_CB_PRESOLVE) {
    //     // std::cout << "presolve" << std::endl;
    // } else if (where == GRB_CB_BARRIER) {
    //     // std::cout << "barrier" << std::endl;
    // } else if (where == GRB_CB_MIP) {
    //     // std::cout << "mip" << std::endl;
    // } else if (where == GRB_CB_MIPNODE) {
    //     std::cout << "===================" << std::endl;
    //     std::cout << "===== mipnode =====" << std::endl;
    //     std::cout << "===================" << std::endl;
    // }else if (where == GRB_CB_SIMPLEX) {
    //     // std::cout << "simplex" << std::endl;
    // } else if (where == GRB_CB_MESSAGE) {
    //     // Message callback
    //     std::string msg = getStringInfo(GRB_CB_MSG_STRING);
    //     // std::cout << "message: " << msg << std::endl;
    // }
    // else {
    //     // std::cout << "other" << std::endl;
    // }

    // // std::cout << "found incumbent" << std::endl;
    // double* x_values = getSolution(x_vars, this->num_vars);

    // // std::cout << "Constructing candidate graph..." << std::endl;
    // auto graph_x = construct_candidate_graph_from_x_vars(x_values);
    // auto tc_graph_x = my_graph::digraph();

    // std::map<my_graph::vertex, my_graph::vertex> g_to_tc;
    // std::vector<std::size_t> id_map(boost::num_vertices(graph_x));
    // std::iota(id_map.begin(), id_map.end(), 0u);

    // boost::transitive_closure(graph_x, tc_graph_x,
    //                             boost::make_assoc_property_map(g_to_tc),
    //                             id_map.data());

    // for (auto& e : g_to_tc) tc_graph_x[e.second] = graph_x[e.first];

    // my_graph::digraph complete_net_flow;
    // boost::copy_graph(problem_instance.input_graph, complete_net_flow);
    // for (const auto& e :
    //         boost::make_iterator_range(boost::edges(complete_net_flow))) {
    //     int var_idx = complete_net_flow[e].id;
    //     complete_net_flow[e].capacity = x_values[var_idx];
    //     if (complete_net_flow[e].capacity <= 1e-6) {
    //         complete_net_flow[e].capacity = 0;
    //     }
    // }

    // for (const auto& e : boost::make_iterator_range(
    //             boost::edges(problem_instance.covering_graph))) {
    //     auto head = boost::source(e, problem_instance.covering_graph);
    //     auto tail = boost::target(e, problem_instance.covering_graph);
    //     if (not boost::edge(head, tail, tc_graph_x).second and
    //         problem_instance.predecessors.at(head).size() != 0u and
    //         problem_instance.predecessors.at(tail).size() != 0u) {

    //         auto set_q = std::set<my_graph::vertex>();
    //         set_q.insert(problem_instance.root);
    //         set_q.insert(problem_instance.predecessors.at(head).begin(),
    //                         problem_instance.predecessors.at(head).end());
    //         set_q.insert(problem_instance.sucessors.at(tail).begin(),
    //                         problem_instance.sucessors.at(tail).end());

    //         my_graph::digraph net_flow;
    //         boost::copy_graph(complete_net_flow, net_flow);

    //         for (const auto& v :
    //                 boost::make_iterator_range(boost::vertices(net_flow))) {
    //             if (set_q.count(v)) {
    //                 boost::clear_vertex(v, net_flow);
    //             }
    //         }

    //         auto f = ajns::flow(net_flow);
    //         f.run(head, tail, ajns::BOYKOL);

    //         if (f.get_max_flow_value() < 1) {
    //             auto min_cut = f.get_min_cut();

    //             if (!min_cut.empty()) {
    //                 GRBLinExpr lhside_ineq = 0;
    //                 for (const auto& cut_edge : min_cut) {
    //                     int var_idx = problem_instance.input_graph[cut_edge].id;
    //                     lhside_ineq += x_vars[var_idx];
    //                 }
    //                 addCut(lhside_ineq >= 1);
    //             }
    //         }
    //     }
    // }
    // delete[] x_values;
// } catch (GRBException e) {
//     std::cout << "Error number: " << e.getErrorCode() << std::endl;
//     std::cout << e.getMessage() << std::endl;
// } catch (...) {
//     std::cout << "Error during callback" << std::endl;
// }
}