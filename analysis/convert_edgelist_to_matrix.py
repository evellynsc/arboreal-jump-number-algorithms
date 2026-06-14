import argparse
import os
import polars as pl
import numpy as np


def edgelist_to_adj_matrix_with_root(df: pl.DataFrame):
    """
    Converts an edge list DataFrame into an adjacency matrix and adds a new root node.

    The new root node will have edges pointing to all nodes that were sources
    (in-degree of 0) in the original graph.

    Args:
        df (pl.DataFrame): A DataFrame with two columns, representing predecessor and successor nodes.

    Returns:
        tuple[np.ndarray, dict]: A tuple containing:
            - The final adjacency matrix with the new root node at index 0.
            - A mapping from the new matrix indices back to the original node IDs.
    """
    # 1. Find all unique nodes and create a mapping to 0-indexed integers
    all_nodes = pl.concat([df['Predecessor'], df['Successor']]).unique().sort()
    node_to_idx = {node_id: i for i, node_id in enumerate(all_nodes)}
    num_original_nodes = len(node_to_idx)

    # 2. Create the initial adjacency matrix for the original graph
    adj_matrix = np.zeros((num_original_nodes, num_original_nodes), dtype=int)

    # 3. Populate the matrix using the 0-indexed mapping
    for pred, succ in df.iter_rows():
        u = node_to_idx[pred]
        v = node_to_idx[succ]
        adj_matrix[u, v] = 1

    # 4. Identify source nodes in the original graph (in-degree == 0)
    in_degrees = np.sum(adj_matrix, axis=0)
    source_nodes_indices = np.where(in_degrees == 0)[0]

    # 5. Create the new, larger matrix for the graph with the added root
    num_new_nodes = num_original_nodes + 1
    new_adj_matrix = np.zeros((num_new_nodes, num_new_nodes), dtype=int)

    # 6. Copy the old adjacency matrix into the new one, offset by 1
    # The new root is at index 0. Original nodes are shifted by 1.
    new_adj_matrix[1:, 1:] = adj_matrix

    # 7. Add edges from the new root (index 0) to the original source nodes
    for source_idx in source_nodes_indices:
        new_adj_matrix[0, source_idx + 1] = 1

    # 8. Create a map from new indices back to original IDs for clarity
    idx_to_node = {i + 1: node_id for node_id, i in node_to_idx.items()}
    idx_to_node[0] = 'NEW_ROOT'

    return new_adj_matrix, idx_to_node


def save_matrix_to_rcp(matrix: np.ndarray, output_path: str):
    """
    Saves an adjacency matrix to a file in the .rcp format.

    The format consists of a header line with the number of nodes,
    followed by the matrix rows with space-separated values.

    Args:
        matrix (np.ndarray): The adjacency matrix to save.
        output_path (str): The path where the file will be saved.
    """
    num_nodes = matrix.shape[0]
    with open(output_path, 'w') as f:
        # Write header: <num_nodes>,n
        f.write(f"{num_nodes},n\n")

        # Write matrix rows
        for row in matrix:
            # Convert each integer in the row to a string and join with spaces
            f.write(" ".join(map(str, row)) + "\n")


def main():
    """Main function to parse arguments and run the conversion."""
    parser = argparse.ArgumentParser(description="Convert a graph edge list (CSV) to an adjacency matrix with an added root.")
    parser.add_argument("input_file", help="The path to the input CSV file containing the edge list.")
    parser.add_argument("output_file", help="The path to save the output .rcp file.")
    args = parser.parse_args()

    if not os.path.isfile(args.input_file):
        print(f"Error: File not found at '{args.input_file}'")
        return

    print(f"Reading graph from: {os.path.basename(args.input_file)}")

    # Read the CSV and strip whitespace from column names (e.g., " Successor")
    df = pl.read_csv(args.input_file)
    df.columns = [col.strip() for col in df.columns]

    if 'Predecessor' not in df.columns or 'Successor' not in df.columns:
        print("Error: CSV must contain 'Predecessor' and 'Successor' columns.")
        return

    final_matrix, idx_map = edgelist_to_adj_matrix_with_root(df)

    save_matrix_to_rcp(final_matrix, args.output_file)
    print(f"\nSuccessfully saved adjacency matrix to: {args.output_file}")
    print(f"  - Matrix shape: {final_matrix.shape}")
    print("  - A new root node was added at index 0.")
    print("\nMapping from matrix index to original node ID:")
    print(idx_map)

if __name__ == "__main__":
    main()
