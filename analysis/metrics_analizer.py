import polars as pl
import sys

def load_and_analyze(csv_path: str = "sop_df.csv"):
    """
    Load experimental data and provide comprehensive algorithm comparison.
    
    Status codes:
    - 2: OPTIMAL (solved)
    - 9: TIME_LIMIT (timeout or suboptimal)
    """
    
    # Load data
    df = pl.read_csv(csv_path)
    
    print("="*80)
    print("ALGORITHM COMPARISON: JUMP NUMBER PROBLEM")
    print("="*80)
    print(f"\nDataset: {csv_path}")
    print(f"Total runs: {len(df)}")
    print(f"Algorithms: {df['algorithm'].unique().to_list()}")
    print(f"Instances: {df['instance'].n_unique()}")
    
    # 1. OVERALL STATISTICS BY ALGORITHM
    print("\n" + "="*80)
    print("1. OVERALL STATISTICS BY ALGORITHM")
    print("="*80)
    
    algo_stats = (
        df.groupby("algorithm").agg([
            pl.col("num_jumps").mean().alias("avg_jumps"),
            pl.col("solve_time").mean().alias("avg_solve_time"),
            pl.col("solve_time").max().alias("max_solve_time"),
            pl.col("solve_time").min().alias("min_solve_time"),
            (pl.col("status") == 2).sum().alias("optimal_count"),
            pl.col("status").count().alias("total_runs"),
        ]).with_columns([
            (pl.col("optimal_count") / pl.col("total_runs") * 100).alias("success_rate_%")
        ]).sort("success_rate_%", descending=True)
    )
    
    print(algo_stats)
    
    # 2. QUALITY: SOLUTION QUALITY (num_jumps)
    print("\n" + "="*80)
    print("2. SOLUTION QUALITY COMPARISON (num_jumps)")
    print("="*80)
    
    quality = (
        df.filter(pl.col("status") == 2)
        .groupby("algorithm").agg([
            pl.col("num_jumps").mean().alias("avg_jumps"),
            pl.col("num_jumps").min().alias("best_jumps"),
            pl.col("num_jumps").max().alias("worst_jumps"),
            pl.col("num_jumps").std().alias("std_jumps"),
        ]).sort("avg_jumps")
    )
    
    print(quality)
    
    # 3. SPEED: SOLVE TIME
    print("\n" + "="*80)
    print("3. SPEED COMPARISON (solve_time in seconds)")
    print("="*80)
    
    speed = (
        df.filter(pl.col("status") == 2)
        .groupby("algorithm").agg([
            pl.col("solve_time").mean().alias("avg_time_s"),
            pl.col("solve_time").median().alias("median_time_s"),
            pl.col("solve_time").quantile(0.9).alias("p90_time_s"),
            pl.col("solve_time").quantile(0.95).alias("p95_time_s"),
        ]).sort("avg_time_s")
    )
    
    print(speed)
    
    # 4. RELIABILITY: TIMEOUT ANALYSIS
    print("\n" + "="*80)
    print("4. RELIABILITY: TIMEOUT ANALYSIS")
    print("="*80)
    
    timeouts = (
        df.groupby("algorithm").agg([
            (pl.col("status") != 2).sum().alias("timeouts"),
            pl.col("status").count().alias("total"),
        ]).with_columns([
            (pl.col("timeouts") / pl.col("total") * 100).alias("timeout_rate_%")
        ]).sort("timeout_rate_%")
    )
    
    print(timeouts)
    
    # 5. PER-INSTANCE COMPARISON
    print("\n" + "="*80)
    print("5. PER-INSTANCE WINNER (best num_jumps)")
    print("="*80)
    
    per_instance = (
        df.groupby("instance").agg([
            pl.col("algorithm"),
            pl.col("num_jumps"),
            pl.col("solve_time"),
            pl.col("status"),
        ])
        .with_columns([
            pl.col("num_jumps").min().alias("best_jumps")
        ])
        .filter(pl.col("num_jumps") == pl.col("best_jumps"))
        .select(["instance", "algorithm", "best_jumps", "solve_time", "status"])
        .sort("instance")
    )
    
    print(per_instance)
    
    # 6. ALGORITHM WINS
    print("\n" + "="*80)
    print("6. ALGORITHM WINS (count of instances where best solution found)")
    print("="*80)
    
    winners = per_instance.groupby("algorithm").count().sort("count", descending=True)
    print(winners)
    
    # 7. INSTANCE DIFFICULTY ANALYSIS
    print("\n" + "="*80)
    print("7. INSTANCE DIFFICULTY (by solve time with FEASIBILITY algo)")
    print("="*80)
    
    difficulty = (
        df.filter(pl.col("algorithm") == "FEASIBILITY")
        .select(["instance", "num_jumps", "solve_time", "status"])
        .sort("solve_time", descending=True)
        .head(10)
    )
    
    print("Top 10 hardest instances (by FEASIBILITY solve time):")
    print(difficulty)
    
    # 8. SUMMARY RECOMMENDATION
    print("\n" + "="*80)
    print("8. RECOMMENDATION")
    print("="*80)
    
    best_algo = algo_stats.row(0, named=True)
    print(f"\n✓ BEST OVERALL ALGORITHM: {best_algo['algorithm']}")
    print(f"  - Success rate: {best_algo['success_rate_%']:.1f}%")
    print(f"  - Average solve time: {best_algo['avg_solve_time']:.3f}s")
    print(f"  - Average jumps: {best_algo['avg_jumps']:.2f}")
    
    # Detailed recommendation
    feas_stats = algo_stats.filter(pl.col("algorithm") == "FEASIBILITY").row(0, named=True)
    multi_stats = algo_stats.filter(pl.col("algorithm") == "MULTIFLOW").row(0, named=True)
    
    print("\nDETAILED COMPARISON:")
    print(f"\nFEASIBILITY:")
    print(f"  Success: {feas_stats['success_rate_%']:.1f}% | Avg time: {feas_stats['avg_solve_time']:.3f}s")
    
    print(f"\nMULTIFLOW:")
    print(f"  Success: {multi_stats['success_rate_%']:.1f}% | Avg time: {multi_stats['avg_solve_time']:.3f}s")
    
    print("\n" + "="*80)

if __name__ == "__main__":
    csv_file = sys.argv[1] if len(sys.argv) > 1 else "sop_df.csv"
    load_and_analyze(csv_file)