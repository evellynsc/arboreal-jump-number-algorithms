
import os
import argparse
import numpy as np
import polars as pl


def transitive_closure(adj_matrix: list[list[int]] | np.ndarray) -> np.ndarray:
    """
    Calculates the transitive closure of a Directed Acyclic Graph (DAG).

    The transitive closure of a DAG is another graph with the same set of vertices
    but with edges added such that if there is a path from vertex u to vertex v
    in the original graph, there is a direct edge from u to v in the transitive closure.
    This algorithm assumes the input graph is acyclic.

    Args:
        adj_matrix (np.ndarray or list of lists): The adjacency matrix of the DAG.
                                                  It must be a square matrix where
                                                  adj_matrix[i][j] = 1 if there is an
                                                  edge from node i to node j, and 0 otherwise.

    Returns:
        np.ndarray: The adjacency matrix of the transitive closure of the graph.
    """
    adj_matrix = np.array(adj_matrix, dtype=int)
    num_nodes = adj_matrix.shape[0]
    if adj_matrix.shape != (num_nodes, num_nodes):
        raise ValueError("The adjacency matrix must be square.")

    # Using a variation of the Floyd-Warshall algorithm to compute transitive closure
    closure_matrix = adj_matrix.copy()
    for k in range(num_nodes):
        for i in range(num_nodes):
            for j in range(num_nodes):
                # If there is a path from i to k and from k to j,
                # then there is a path from i to j.
                if closure_matrix[i, k] and closure_matrix[k, j]:
                    closure_matrix[i, j] = 1

    return closure_matrix   

def transitive_reduction(adj_matrix: list[list[int]] | np.ndarray) -> np.ndarray:
    """
    Calculates the transitive reduction of a Directed Acyclic Graph (DAG).

    The transitive reduction of a DAG is another graph with the same set of vertices
    but with the minimum number of edges that preserves the original graph's
    reachability. It is found by removing every edge (u, v) for which there is
    a path of length > 1 from u to v.

    This algorithm assumes the input graph is acyclic.

    Args:
        adj_matrix (np.ndarray or list of lists): The adjacency matrix of the DAG.
                                                  It must be a square matrix where
                                                  adj_matrix[i][j] = 1 if there is an
                                                  edge from node i to node j, and 0 otherwise.

    Returns:
        np.ndarray: The adjacency matrix of the transitive reduction of the graph.
    """
    adj_matrix = np.array(adj_matrix, dtype=int)
    num_nodes = adj_matrix.shape[0]
    if adj_matrix.shape != (num_nodes, num_nodes):
        raise ValueError("The adjacency matrix must be square.")

    # 1. Calculate the transitive closure of the graph.
    # An edge (i, j) exists in the transitive closure if there is a path
    # of length 1 or more from i to j.
    # We use a variation of the Floyd-Warshall algorithm for this.
    transitive_closure = transitive_closure(adj_matrix)

    # 2. Identify and remove redundant edges.
    # An edge (i, j) is redundant if there is a path from i to j with length > 1.
    # This is equivalent to the existence of an intermediate node k such that
    # there is an edge (i, k) and a path from k to j.
    reduced_matrix = adj_matrix.copy()
    for i in range(num_nodes):
        for j in range(num_nodes):
            # If there is no direct edge (i, j), there's nothing to do.
            if adj_matrix[i, j] == 0:
                continue

            # Check for the existence of a longer path
            for k in range(num_nodes):
                # Is there a path i -> k -> ... -> j?
                # For this, we need an edge (i, k) and a path (k, j).
                # The existence of the path (k, j) is given by the transitive closure.
                if adj_matrix[i, k] and transitive_closure[k, j]:
                    # We found a longer path. The direct edge (i, j) is redundant.
                    reduced_matrix[i, j] = 0
                    break  # Move to the next edge to check

    return reduced_matrix



def calculate_instances_invariants(file_path):
    """
    Analyzes a graph file to calculate node and edge statistics.

    Args:
        file_path (str): The path to the graph file.
    """
    with open(file_path, 'r') as f:
        lines = f.readlines()

    num_nodes_str = lines[0].split(',')[0]
    num_nodes = int(num_nodes_str)

    adj_matrix = []
    for line in lines[1:num_nodes + 1]:
        row = [int(x) for x in line.strip().split()]
        adj_matrix.append(row)


    reduction_adj_matrix = np.array(adj_matrix)

    reduction_adj_matrix = transitive_reduction(adj_matrix=reduction_adj_matrix)

    num_edges = np.sum(reduction_adj_matrix)
    
    in_degrees = np.sum(reduction_adj_matrix, axis=0)
    
    nodes_indegree_gt_one = np.sum(in_degrees > 1)

    closed_adj_matrix = np.array(adj_matrix)
    closed_adj_matrix = transitive_closure(adj_matrix=closed_adj_matrix)
    num_closed_edges = np.sum(closed_adj_matrix)

    return {
        "num_nodes": num_nodes,
        "num_edges": num_edges,
        "num_closed_edges": num_closed_edges,
        "in_degrees": in_degrees,
        "nodes_indegree_gt_one": nodes_indegree_gt_one
    }

def main():
    """
    Main function to parse command-line arguments and analyze graph files.
    """
    parser = argparse.ArgumentParser(description="Analyze graph files in a directory.")
    parser.add_argument("directory", help="The path to the directory containing graph files.")
    args = parser.parse_args()

    directory = args.directory
    if not os.path.isdir(directory):
        print(f"Error: Directory not found at '{directory}'")
        return
    instance_invariants = []
    for filename in os.listdir(directory):
        if filename.endswith(".rcp") or filename.endswith(".txt"):
            print("Analyzing file:", filename)
            file_path = os.path.join(directory, filename)
            result = calculate_instances_invariants(file_path)
            result["instance"] = filename
            instance_invariants.append(result)
    df = pl.DataFrame(instance_invariants)
    print(df)

if __name__ == "__main__":
    main()
