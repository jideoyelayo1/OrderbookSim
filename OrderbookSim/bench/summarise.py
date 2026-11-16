"""Summarise orderbook_bench output: median and IQR across repetitions.

usage: python3 summarise.py RESULTS_DIR   (prints Markdown to stdout; standard library only)
"""
import csv
import statistics
import sys
from collections import defaultdict
from pathlib import Path


def med_iqr(values):
    """Median and inter-quartile range (inclusive method) across repetitions."""
    q1, med, q3 = statistics.quantiles(values, n=4, method="inclusive")
    return med, q1, q3


def fmt(values, unit=""):
    med, q1, q3 = med_iqr(values)
    return f"{med:,.0f}{unit} [{q1:,.0f}–{q3:,.0f}]"


def main(out):
    out = Path(out)
    wl = {r["key"]: r["value"] for r in csv.DictReader(open(out / "workload.csv"))}
    rows = defaultdict(list)
    for r in csv.DictReader(open(out / "latency_per_rep.csv")):
        rows[r["op"]].append(r)
    thr = [float(r["calls_per_second"]) for r in csv.DictReader(open(out / "throughput_per_rep.csv"))]
    mem = {r["key"]: r["value"] for r in csv.DictReader(open(out / "memory.csv"))}
    reps = len(thr)

    print(f"Latency per call in ns: median across {reps} repetitions, [IQR across repetitions].")
    print("`IQR within run` is the median over repetitions of p75 − p25 for that run.\n")
    print("| op | calls per rep | trades per call | p50 | IQR within run | p90 | p99 | p99.9 | max (median of reps) | max (all reps) | allocations per call | bytes allocated per call |")
    print("|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|")
    for op, rs in rows.items():
        col = lambda k: [float(r[k]) for r in rs]
        n = int(rs[0]["n"])
        trades = int(wl.get(f"trades_{op}", 0)) / n
        within = [float(r["p75_ns"]) - float(r["p25_ns"]) for r in rs]
        print(f"| {op} | {n:,} | {trades:.2f} | {fmt(col('p50_ns'))} | {fmt(within)} | {fmt(col('p90_ns'))} | "
              f"{fmt(col('p99_ns'))} | {fmt(col('p999_ns'))} | {fmt(col('max_ns'))} | {max(col('max_ns')):,.0f} | "
              f"{statistics.median(col('allocs_per_op')):.2f} | {statistics.median(col('bytes_per_op')):,.0f} |")
    print()
    med, q1, q3 = med_iqr(thr)
    print(f"Throughput (measured calls, whole block, no per-call timer): median {med:,.0f} calls/s "
          f"[IQR {q1:,.0f}–{q3:,.0f}] over {reps} repetitions.\n")
    print(f"Peak resident set size of the benchmark process: {int(mem['peak_rss_bytes']) / 2**20:,.0f} MiB.\n")
    print("Workload as generated (seed, mix and realised depth):\n")
    print("| key | value |\n|---|---|")
    for k, v in wl.items():
        print(f"| {k} | {v} |")


if __name__ == "__main__":
    main(sys.argv[1])
