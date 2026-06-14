import polars as pl
import seaborn as sns
import matplotlib.pyplot as plt
import numpy as np


def create_sample_dataframe() -> pl.DataFrame:
    """
    Creates a sample Polars DataFrame for demonstration purposes.
    In a real scenario, you would load your actual data, e.g., from a CSV file.
    """
    data = {
        "algorithm": ["MULTIFLOW", "MULTIFLOW", "SINGLEFLOW", "MULTIFLOW", "MULTIFLOW", "MULTIFLOW", "MULTIFLOW"],
        "runtime": [10.5, 25.2, 15.1, 55.9, 80.3, 120.5, 33.1],
        "num_nodes": [20, 30, 20, 50, 50, 80, 30],
        "num_closed_edges": [150, 400, 140, 1200, 1150, 3100, 410],
        "status": ["SUCCESS", "SUCCESS", "SUCCESS", "TIMEOUT", "SUCCESS", "TIMEOUT", "SUCCESS"],
        "instance": ["g1.rcp", "g2.rcp", "g3.rcp", "g4.rcp", "g5.rcp", "g6.rcp", "g7.rcp"]
    }
    return pl.DataFrame(data)


def plot_runtime_vs_density(df: pl.DataFrame):
    """
    Generates a scatter plot of runtime vs. closed edge density,
    filtered for the 'MULTIFLOW' algorithm.

    The plot uses:
    - x-axis: runtime
    - y-axis: closed_edge_density
    - color: varies by num_nodes
    - marker style: varies by status

    Args:
        df (pl.DataFrame): DataFrame containing the analysis results.
                           Must include 'algorithm', 'runtime', 'num_nodes',
                           'num_closed_edges', and 'status' columns.
    """
    # 1. Filter for the 'MULTIFLOW' algorithm
    multiflow_df = df.filter(pl.col("algorithm") == "MULTIFLOW")

    if multiflow_df.is_empty():
        print("No data found for 'MULTIFLOW' algorithm. Cannot generate plot.")
        return

    # 2. Calculate 'closed_edge_density'
    # Density = num_edges / max_possible_edges. For a directed graph, max is n * (n - 1).
    plot_df = multiflow_df.with_columns(
        (
            pl.col("num_closed_edges") / (pl.col("num_nodes") * (pl.col("num_nodes") - 1))
        ).alias("closed_edge_density")
    ).fill_nan(0)  # Handle cases where n<=1, which would cause division by zero

    # --- Add Jitter to Data for Better Visualization ---
    # To prevent points from overlapping in dense clusters, we add a small
    # amount of random noise ("jitter") to their positions.
    num_rows = len(plot_df)
    x_range = plot_df["runtime"].max() - plot_df["runtime"].min()
    y_range = plot_df["closed_edge_density"].max() - plot_df["closed_edge_density"].min()

    # Set jitter strength as a small fraction of the data range (e.g., 2%).
    # Use a conditional to avoid errors if all data points are the same.
    x_jitter = x_range * 0.02 if x_range is not None and x_range > 0 else 0
    y_jitter = y_range * 0.02 if y_range is not None and y_range > 0 else 0

    # Create new jittered columns in a new DataFrame for plotting.
    # The original data remains unchanged.
    plot_df_jittered = plot_df.with_columns([
        (pl.col("runtime") + np.random.uniform(-x_jitter, x_jitter, size=num_rows)).alias("runtime_jittered"),
        (pl.col("closed_edge_density") + np.random.uniform(-y_jitter, y_jitter, size=num_rows)).alias("closed_edge_density_jittered")
    ])

    print("Data to be plotted (with jitter):")
    print(plot_df_jittered)

    # 3. Create the scatter plot using seaborn
    plt.figure(figsize=(12, 8))
    sns.set_theme(style="whitegrid")

    scatter_plot = sns.scatterplot(
        data=plot_df_jittered,
        x="runtime_jittered",
        y="closed_edge_density_jittered",
        hue="num_nodes",      # Color points by number of nodes
        style="status",       # Change marker shape by status
        palette="viridis_r",  # A nice color map for continuous data
        s=150,                # Increase marker size for better visibility
        alpha=0.8             # Add some transparency to complement the jitter
    )

    # 4. Customize the plot
    scatter_plot.set_title("Runtime vs. Closed Edge Density for MULTIFLOW Algorithm", fontsize=16, weight='bold')
    scatter_plot.set_xlabel("Runtime (s)", fontsize=12)
    scatter_plot.set_ylabel("Closed Edge Density", fontsize=12)
    plt.legend(title="Legend", bbox_to_anchor=(1.05, 1), loc='upper left')
    plt.tight_layout()

    # 5. Show the plot
    plt.show()


def plot_grid_by_status(df: pl.DataFrame):
    """
    Generates a grid of scatter plots of runtime vs. closed edge density,
    with a separate subplot for each 'status'.

    The plot uses:
    - x-axis: runtime
    - y-axis: closed_edge_density
    - color: varies by num_nodes
    - columns: separate plots for each status

    Args:
        df (pl.DataFrame): DataFrame containing the analysis results.
                           Must include 'algorithm', 'runtime', 'num_nodes',
                           'num_closed_edges', and 'status' columns.
    """
    # 1. Filter for the 'MULTIFLOW' algorithm
    multiflow_df = df.filter(pl.col("algorithm") == "MULTIFLOW")

    if multiflow_df.is_empty():
        print("No data found for 'MULTIFLOW' algorithm. Cannot generate plot.")
        return

    # 2. Calculate 'closed_edge_density'
    plot_df = multiflow_df.with_columns(
        (
            pl.col("num_closed_edges") / (pl.col("num_nodes") * (pl.col("num_nodes") - 1))
        ).alias("closed_edge_density")
    ).fill_nan(0)

    print("Data to be plotted in grid:")
    print(plot_df)

    # 3. Create the grid of scatter plots using seaborn.relplot
    # relplot is a figure-level function that creates a FacetGrid.
    # The 'col' parameter is key to creating the grid.
    g = sns.relplot(
        data=plot_df,
        x="runtime",
        y="closed_edge_density",
        hue="num_nodes",      # Color points by number of nodes
        col="status",         # Create columns for each status
        kind="scatter",
        palette="viridis_r",
        s=100,
        alpha=0.8
    )

    # 4. Customize the plot
    g.fig.suptitle("Runtime vs. Density by Status for MULTIFLOW Algorithm", y=1.03, fontsize=16, weight='bold')
    g.set_axis_labels("Runtime (s)", "Closed Edge Density")
    g.set_titles("Status: {col_name}") # Customize subplot titles
    plt.tight_layout(rect=[0, 0, 1, 0.97]) # Adjust layout to make space for suptitle

    # 5. Show the plot
    plt.show()


if __name__ == '__main__':
    # For this example, we'll use a sample DataFrame.
    # In your actual use case, you would load your DataFrame from a file.
    # Example: df = pl.read_csv("your_full_results.csv")
    sample_df = create_sample_dataframe()
    # plot_runtime_vs_density(sample_df) # You can still run the old plot if you like
    plot_grid_by_status(sample_df)