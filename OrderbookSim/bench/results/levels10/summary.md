Latency per call in ns: median across 10 repetitions, [IQR across repetitions].
`IQR within run` is the median over repetitions of p75 − p25 for that run.

| op | calls per rep | trades per call | p50 | IQR within run | p90 | p99 | p99.9 | max (median of reps) | max (all reps) | allocations per call | bytes allocated per call |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| add | 547,954 | 0.00 | 167 [167–167] | 84 [84–84] | 333 [333–333] | 520 [500–542] | 896 [709–1,552] | 11,763,896 [10,342,084–15,021,687] | 16,004,667 | 5.29 | 4,521 |
| cancel | 352,069 | 0.00 | 83 [83–83] | 1 [1–1] | 125 [125–125] | 167 [167–208] | 292 [250–292] | 22,458 [16,281–24,802] | 87,208 | 0.00 | 0 |
| match_gtc | 24,899 | 2.93 | 1,542 [1,541–1,542] | 1,041 [1,041–1,042] | 2,625 [2,625–2,656] | 3,687 [3,625–3,781] | 4,958 [4,688–7,844] | 150,354 [140,364–188,885] | 204,208 | 20.65 | 5,122 |
| match_fak | 24,818 | 2.94 | 1,542 [1,542–1,573] | 1,041 [1,041–1,042] | 2,646 [2,625–2,698] | 3,750 [3,676–3,782] | 5,000 [4,688–7,333] | 5,410,062 [4,370,448–6,070,000] | 7,357,958 | 20.72 | 4,722 |
| match_fok | 25,081 | 2.93 | 1,542 [1,542–1,583] | 1,001 [1,001–1,042] | 2,666 [2,666–2,698] | 3,771 [3,750–3,833] | 5,083 [4,802–5,510] | 35,792 [19,750–43,833] | 78,209 | 20.65 | 4,568 |
| match_market | 25,179 | 2.95 | 1,542 [1,542–1,573] | 1,042 [1,041–1,073] | 2,646 [2,625–2,698] | 3,750 [3,718–3,782] | 5,146 [4,625–6,469] | 25,480 [23,094–42,125] | 57,459 | 20.76 | 4,573 |

Throughput (measured calls, whole block, no per-call timer): median 3,091,216 calls/s [IQR 3,058,724–3,133,428] over 10 repetitions.

Peak resident set size of the benchmark process: 167 MiB.

Workload as generated (seed, mix and realised depth):

| key | value |
|---|---|
| seed | 42 |
| reps | 10 |
| prefill_orders | 200 |
| warmup_calls | 100000 |
| measured_calls | 1000000 |
| levels_each_side | 10 |
| target_resting_orders | 200 |
| mean_resting_orders | 129.7 |
| min_resting_orders | 85 |
| max_resting_orders | 177 |
| mean_bid_levels | 8.6 |
| mean_ask_levels | 8.6 |
| final_book_size | 154 |
| clock_resolution_ns | 41 |
| clock_overhead_p50_ns | 0 |
| clock_overhead_p99_ns | 42 |
| generation_seconds | 0.47 |
| calls_add | 547954 |
| trades_add | 0 |
| calls_cancel | 352069 |
| trades_cancel | 0 |
| calls_match_gtc | 24899 |
| trades_match_gtc | 72967 |
| calls_match_gfd | 0 |
| trades_match_gfd | 0 |
| calls_match_fak | 24818 |
| trades_match_fak | 73050 |
| calls_match_fok | 25081 |
| trades_match_fok | 73484 |
| calls_match_market | 25179 |
| trades_match_market | 74343 |
| calls_reject_fak | 0 |
| trades_reject_fak | 0 |
| calls_reject_fok | 0 |
| trades_reject_fok | 0 |
| calls_reject_market | 0 |
| trades_reject_market | 0 |
