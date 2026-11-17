Latency per call in ns: median across 10 repetitions, [IQR across repetitions].
`IQR within run` is the median over repetitions of p75 − p25 for that run.

| op | calls per rep | trades per call | p50 | IQR within run | p90 | p99 | p99.9 | max (median of reps) | max (all reps) | allocations per call | bytes allocated per call |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| add | 549,002 | 0.00 | 208 [208–208] | 125 [84–125] | 396 [375–417] | 625 [594–625] | 833 [792–875] | 13,832,312 [13,047,031–15,717,542] | 18,559,583 | 5.49 | 20,867 |
| cancel | 350,996 | 0.00 | 125 [125–125] | 82 [82–82] | 208 [208–208] | 333 [292–333] | 459 [458–563] | 20,437 [13,938–49,760] | 86,958 | 0.00 | 0 |
| match_gtc | 25,059 | 2.96 | 1,667 [1,667–1,667] | 1,167 [1,166–1,198] | 2,916 [2,875–2,917] | 4,062 [4,042–4,125] | 5,333 [5,250–6,167] | 38,271 [20,396–56,031] | 157,083 | 20.82 | 20,906 |
| match_fak | 24,821 | 2.96 | 1,667 [1,667–1,667] | 1,167 [1,167–1,198] | 2,917 [2,916–2,958] | 4,084 [4,042–4,208] | 5,354 [5,292–7,177] | 60,479 [52,978–147,938] | 196,958 | 20.82 | 21,040 |
| match_fok | 25,147 | 2.96 | 1,708 [1,667–1,708] | 1,167 [1,167–1,167] | 2,958 [2,917–2,959] | 4,125 [4,125–4,240] | 5,396 [5,240–6,052] | 50,833 [20,969–76,167] | 104,000 | 20.81 | 20,916 |
| match_market | 24,975 | 2.95 | 1,667 [1,666–1,667] | 1,167 [1,166–1,198] | 2,916 [2,875–2,917] | 4,063 [4,041–4,198] | 5,438 [5,239–5,979] | 163,916 [136,938–173,427] | 572,375 | 20.76 | 21,419 |

Throughput (measured calls, whole block, no per-call timer): median 2,597,382 calls/s [IQR 2,547,452–2,627,365] over 10 repetitions.

Peak resident set size of the benchmark process: 170 MiB.

Workload as generated (seed, mix and realised depth):

| key | value |
|---|---|
| seed | 42 |
| reps | 10 |
| prefill_orders | 1000 |
| warmup_calls | 100000 |
| measured_calls | 1000000 |
| levels_each_side | 50 |
| target_resting_orders | 1000 |
| mean_resting_orders | 640.3 |
| min_resting_orders | 537 |
| max_resting_orders | 772 |
| mean_bid_levels | 37.9 |
| mean_ask_levels | 38.0 |
| final_book_size | 591 |
| clock_resolution_ns | 41 |
| clock_overhead_p50_ns | 0 |
| clock_overhead_p99_ns | 42 |
| generation_seconds | 0.54 |
| calls_add | 549002 |
| trades_add | 0 |
| calls_cancel | 350996 |
| trades_cancel | 0 |
| calls_match_gtc | 25059 |
| trades_match_gtc | 74249 |
| calls_match_gfd | 0 |
| trades_match_gfd | 0 |
| calls_match_fak | 24821 |
| trades_match_fak | 73562 |
| calls_match_fok | 25147 |
| trades_match_fok | 74486 |
| calls_match_market | 24975 |
| trades_match_market | 73709 |
| calls_reject_fak | 0 |
| trades_reject_fak | 0 |
| calls_reject_fok | 0 |
| trades_reject_fok | 0 |
| calls_reject_market | 0 |
| trades_reject_market | 0 |
