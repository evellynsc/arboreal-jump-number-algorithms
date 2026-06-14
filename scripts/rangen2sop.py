import sys

def convert_rcpsp_to_rooted_adj_matrix(input_file_path: str, output_file_path: str):
    """
    Reads an RCPSP file, builds its precedence graph, adds a new universal
    root node, and writes the result as an adjacency matrix.

    Args:
        input_file_path (str): The path to the source RCPSP file.
        output_file_path (str): The path where the output adjacency matrix
                                file will be saved.
    """
    try:
        with open(input_file_path, 'r') as f:
            lines = [line.strip() for line in f.readlines() if line.strip()]
    except FileNotFoundError:
        print(f"Error: Input file not found at '{input_file_path}'")
        return

    # --- 1. Parse and Build the Original Graph ---
    header = lines[0].split()
    original_num_activities = int(header[0])
    num_resources = int(header[1])

    original_adj_matrix = [[0] * original_num_activities for _ in range(original_num_activities)]
    activity_lines = lines[2:2 + original_num_activities]

    for i, line in enumerate(activity_lines):
        predecessor_num = i + 1
        parts = [int(p) for p in line.split()]
        
        if num_resources == 1:
            num_successors_idx = 2
        else:
            num_successors_idx = 1 + num_resources

        if parts[num_successors_idx] > 0:
            successors = parts[num_successors_idx + 1:]
            for succ_num in successors:
                predecessor_idx = predecessor_num - 1
                successor_idx = succ_num - 1
                original_adj_matrix[predecessor_idx][successor_idx] = 1

    # --- 2. Add the New Root Node ---
    new_num_vertices = original_num_activities + 1
    # Initialize a new, larger matrix for the rooted graph
    rooted_adj_matrix = [[0] * new_num_vertices for _ in range(new_num_vertices)]

    # The new root (at index 0) has an edge to all original nodes
    for i in range(1, new_num_vertices):
        rooted_adj_matrix[0][i] = 1

    # Embed the original graph into the new matrix, offset by 1
    for i in range(original_num_activities):
        for j in range(original_num_activities):
            rooted_adj_matrix[i + 1][j + 1] = original_adj_matrix[i][j]

    # --- 3. Write the Final Output File ---
    with open(output_file_path, 'w') as f:
        f.write(f"{new_num_vertices},n\n")
        for row in rooted_adj_matrix:
            f.write(' '.join(map(str, row)) + '\n')

    print(f"✅ Successfully converted '{input_file_path}' to '{output_file_path}' with a new root.")


if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Usage: python rangen2sop.py <input_rcpsp_file> <output_adj_matrix_file>")
        sys.exit(1)

    input_path = sys.argv[1]
    output_path = sys.argv[2]
    convert_rcpsp_to_rooted_adj_matrix(input_path, output_path)