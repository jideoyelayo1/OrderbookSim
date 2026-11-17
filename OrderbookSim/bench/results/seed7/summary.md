Latency per call in ns: median across 10 repetitions, [IQR across repetitions].
`IQR within run` is the median over repetitions of p75 − p25 for that run.

| op | calls per rep | trades per call | p50 | IQR within run | p90 | p99 | p99.9 | max (median of reps) | max (all reps) | allocations per call | bytes allocated per call |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| add | 549,100 | 0.00 | 208 [208–208] | 84 [84–84] | 375 [375–375] | 584 [584–584] | 792 [792–865] | 13,570,292 [13,078,489–14,905,729] | 18,698,000 | 5.49 | 20,839 |
| cancel | 351,191 | 0.00 | 125 [125–125] | 82 [82–82] | 208 [208–208] | 292 [292–292] | 458 [458–459] | 17,834 [8,479–35,656] | 72,208 | 0.00 | 0 |
| match_gtc | 25,054 | 2.97 | 1,666 [1,625–1,667] | 1,146 [1,125–1,166] | 2,875 [2,844–2,916] | 4,041 [4,000–4,073] | 5,167 [5,000–5,198] | 42,874 [40,614–51,416] | 103,333 | 20.84 | 20,985 |
| match_fak | 24,905 | 2.95 | 1,646 [1,625–1,666] | 1,125 [1,094–1,156] | 2,875 [2,834–2,875] | 4,083 [4,041–4,114] | 5,313 [5,219–5,406] | 64,000 [61,469–79,312] | 143,708 | 20.77 | 21,083 |
| match_fok | 24,942 | 2.98 | 1,688 [1,667–1,708] | 1,126 [1,125–1,166] | 2,896 [2,875–2,917] | 4,104 [4,052–4,125] | 5,374 [5,333–5,479] | 135,396 [131,688–157,844] | 206,541 | 20.89 | 21,291 |
| match_market | 24,808 | 2.96 | 1,666 [1,635–1,667] | 1,125 [1,125–1,125] | 2,854 [2,833–2,875] | 4,062 [4,010–4,114] | 5,271 [5,208–5,573] | 175,021 [156,334–241,666] | 1,062,375 | 20.79 | 21,326 |

Throughput (measured calls, whole block, no per-call timer): median 2,731,103 calls/s [IQR 2,672,740–2,757,988] over 10 repetitions.

Peak resident set size of the benchmark process: 164 MiB.

Workload as generated (seed, mix and realised depth):

| key | value |
|---|---|
| seed | 7 |
| reps | 10 |
| prefill_orders | 1000 |
| warmup_calls | 100000 |
| measured_calls | 1000000 |
| levels_each_side | 50 |
| target_resting_orders | 1000 |
| mean_resting_orders | 639.9 |
| min_resting_orders | 546 |
| max_resting_orders | 751 |
| mean_bid_levels | 37.9 |
| mean_ask_levels | 38.1 |
| final_book_size | 692 |
| clock_resolution_ns | 41 |
| clock_overhead_p50_ns | 0 |
| clock_overhead_p99_ns | 42 |
| generation_seconds | 0.56 |
| calls_add | 549100 |
| trades_add | 0 |
| calls_cancel | 351191 |
| trades_cancel | 0 |
| calls_match_gtc | 25054 |
| trades_match_gtc | 74375 |
| calls_match_gfd | 0 |
| trades_match_gfd | 0 |
| calls_match_fak | 24905 |
| trades_match_fak | 73547 |
| calls_match_fok | 24942 |
| trades_match_fok | 74286 |
| calls_match_market | 24808 |
| trades_match_market | 73372 |
| calls_reject_fak | 0 |
| trades_reject_fak | 0 |
| calls_reject_fok | 0 |
| trades_reject_fok | 0 |
| calls_reject_market | 0 |
| trades_reject_market | 0 |
