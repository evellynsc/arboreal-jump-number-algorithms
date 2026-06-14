import argparse
import os
import numpy as np

# Add imports for graph creation and visualization
import networkx as nx
import matplotlib.pyplot as plt


def transitive_closure(adj_matrix: list[list[int]] | np.ndarray) -> np.ndarray:
    """
    Calculates the transitive closure of a Directed Acyclic Graph (DAG).

    Args:
        adj_matrix (np.ndarray or list of lists): The adjacency matrix of the DAG.

    Returns:
        np.ndarray: The boolean adjacency matrix of the transitive closure.
    """
    adj_matrix = np.array(adj_matrix, dtype=bool)
    num_nodes = adj_matrix.shape[0]
    if adj_matrix.shape != (num_nodes, num_nodes):
        raise ValueError("The adjacency matrix must be square.")

    # Use a Floyd-Warshall-like algorithm, vectorized for efficiency.
    closure_matrix = adj_matrix.copy()
    for k in range(num_nodes):
        # For each intermediate vertex k, if there is a path from i to k and k to j,
        # there is a path from i to j.
        closure_matrix |= closure_matrix[:, k:k+1] @ closure_matrix[k:k+1, :]

    return closure_matrix


def generate_independence_graph(adj_matrix: list[list[int]] | np.ndarray):
    """
    Generates a new graph based on node independence in a given DAG.

    The new graph is constructed based on the following rules:
    1. A node from the original graph is included in the new graph if its
       in-degree is greater than 1.
    2. An undirected edge exists between two nodes in the new graph if they
       are "independent" in the original graph.
    3. Two nodes are defined as independent if they do not share any common
       ancestors (including themselves).

    Args:
        adj_matrix (np.ndarray or list of lists): The adjacency matrix of the original DAG.

    Returns:
        tuple[np.ndarray, list[int]]: A tuple containing:
            - The adjacency matrix of the new undirected independence graph.
            - A list of the original node indices included in the new graph.
    """
    adj_matrix = np.array(adj_matrix, dtype=int)
    num_nodes = adj_matrix.shape[0]

    # Rule 1: Identify nodes for the new graph (in-degree > 1)
    in_degrees = np.sum(adj_matrix, axis=0)
    nodes_to_include_indices = np.where(in_degrees > 1)[0]

    num_new_nodes = len(nodes_to_include_indices)
    if num_new_nodes < 2:
        # Not enough nodes to form an edge
        return np.array([[]] * num_new_nodes, dtype=int), nodes_to_include_indices.tolist()

    # Rule 3: Define independence by finding ancestor sets
    # First, get the transitive closure for paths of length >= 1
    tc_matrix = transitive_closure(adj_matrix)
    # An ancestor set includes the node itself. Add the identity matrix.
    ancestor_matrix = tc_matrix | np.identity(num_nodes, dtype=bool)
    # Now, ancestor_matrix[i, j] is true if i is an ancestor of j.
    # The ancestors of a node j are represented by the j-th column.

    # Rule 2: Build the new graph by linking independent nodes
    new_adj_matrix = np.zeros((num_new_nodes, num_new_nodes), dtype=int)

    # Iterate through all unique pairs of nodes in the new graph
    for i in range(num_new_nodes):
        for j in range(i + 1, num_new_nodes):
            # Get the indices of the nodes in the original graph
            original_idx_a = nodes_to_include_indices[i]
            original_idx_b = nodes_to_include_indices[j]

            # Get the ancestor sets (columns from the ancestor matrix)
            ancestors_a = ancestor_matrix[:, original_idx_a]
            ancestors_b = ancestor_matrix[:, original_idx_b]

            # Check for independence: True if the intersection of ancestor sets is empty
            if not np.any(ancestors_a & ancestors_b):
                # They are independent, add an undirected edge
                new_adj_matrix[i, j] = 1
                new_adj_matrix[j, i] = 1

    return new_adj_matrix, nodes_to_include_indices.tolist()


def draw_graph_from_adj_matrix(adj_matrix: np.ndarray, node_labels: list = None, title: str = "Graph"):
    """
    Creates and draws a NetworkX graph from an adjacency matrix.

    Args:
        adj_matrix (np.ndarray): The adjacency matrix of the graph.
        node_labels (list, optional): A list of labels for the nodes. If provided,
                                      the graph nodes will be relabeled. Defaults to None.
        title (str, optional): The title for the plot. Defaults to "Graph".
    """
    # Determine if the graph is directed or undirected by checking if the matrix is symmetric
    is_directed = not np.allclose(adj_matrix, adj_matrix.T)
    graph_type = nx.DiGraph if is_directed else nx.Graph

    # Create a NetworkX graph from the numpy adjacency matrix
    G = nx.from_numpy_array(adj_matrix, create_using=graph_type)

    # Relabel nodes if custom labels are provided (e.g., from the original graph)
    if node_labels:
        labels = {i: label for i, label in enumerate(node_labels)}
        nx.relabel_nodes(G, labels, copy=False)

    # Draw the graph using matplotlib
    plt.figure(figsize=(10, 8))
    # Use a seed for the layout algorithm for reproducible visualizations
    pos = nx.spring_layout(G, seed=42)
    nx.draw(G, pos, with_labels=True, node_color='skyblue', node_size=800, edge_color='gray', font_size=12, width=1.5)
    plt.title(title, fontsize=16)
    plt.show()


def main():
    """Main function to parse command-line arguments and analyze a graph file."""
    parser = argparse.ArgumentParser(description="Generate an independence graph from a DAG file.")
    parser.add_argument("file", help="The path to the graph file.")
    args = parser.parse_args()

    file_path = args.file
    if not os.path.isfile(file_path):
        print(f"Error: File not found at '{file_path}'")
        return

    print(f"Reading graph from: {os.path.basename(file_path)}")

    with open(file_path, 'r') as f:
        lines = f.readlines()
    num_nodes_str = lines[0].split(',')[0]
    num_nodes = int(num_nodes_str)
    adj_matrix = np.array([[int(x) for x in line.strip().split()] for line in lines[1:num_nodes + 1]])

    independence_adj_matrix, new_graph_nodes = generate_independence_graph(adj_matrix)

    print("\n--- Independence Graph Results ---")
    print(f"Original nodes included (nodes with in-degree > 1): {new_graph_nodes}")
    print(f"New graph has {independence_adj_matrix.shape[0]} nodes and {np.sum(independence_adj_matrix) // 2} edges.")
    print("\nAdjacency Matrix of Independence Graph:")
    print(independence_adj_matrix)

    # Visualize the resulting independence graph if it's not empty
    if independence_adj_matrix.shape[0] > 0:
        print("\nAttempting to visualize the Independence Graph...")
        draw_graph_from_adj_matrix(
            independence_adj_matrix,
            node_labels=new_graph_nodes,
            title=f"Independence Graph for {os.path.basename(file_path)}"
        )


if __name__ == "__main__":
    main()