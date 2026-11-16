Latency per call in ns: median across 10 repetitions, [IQR across repetitions].
`IQR within run` is the median over repetitions of p75 − p25 for that run.

| op | calls per rep | trades per call | p50 | IQR within run | p90 | p99 | p99.9 | max (median of reps) | max (all reps) | allocations per call | bytes allocated per call |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| add | 549,214 | 0.00 | 375 [375–375] | 167 [166–167] | 584 [584–615] | 833 [833–834] | 1,083 [1,042–1,115] | 14,053,854 [13,061,822–15,973,667] | 17,689,500 | 5.64 | 204,403 |
| cancel | 350,544 | 0.00 | 292 [291–292] | 167 [166–167] | 500 [500–500] | 792 [792–834] | 1,166 [1,125–1,208] | 40,104 [26,864–53,781] | 164,625 | 0.00 | 0 |
| match_gtc | 25,089 | 2.96 | 1,958 [1,958–1,959] | 1,333 [1,333–1,333] | 3,396 [3,375–3,417] | 4,958 [4,916–5,000] | 6,896 [6,802–7,136] | 385,271 [300,187–448,844] | 1,150,084 | 20.80 | 205,409 |
| match_fak | 24,989 | 2.96 | 1,958 [1,958–1,959] | 1,333 [1,333–1,334] | 3,396 [3,375–3,417] | 5,000 [4,927–5,083] | 7,104 [6,968–7,156] | 45,271 [29,229–89,323] | 347,292 | 20.79 | 204,494 |
| match_fok | 25,163 | 2.97 | 2,000 [2,000–2,000] | 1,354 [1,333–1,375] | 3,479 [3,458–3,500] | 5,146 [5,094–5,240] | 7,083 [6,875–7,167] | 104,980 [70,521–125,990] | 766,500 | 20.86 | 204,679 |
| match_market | 25,001 | 2.96 | 1,959 [1,958–2,000] | 1,333 [1,302–1,334] | 3,416 [3,385–3,448] | 5,042 [4,968–5,104] | 7,000 [6,823–7,260] | 156,146 [143,635–191,218] | 1,513,833 | 20.81 | 204,851 |

Throughput (measured calls, whole block, no per-call timer): median 1,791,210 calls/s [IQR 1,783,234–1,826,432] over 10 repetitions.

Peak resident set size of the benchmark process: 171 MiB.

Workload as generated (seed, mix and realised depth):

| key | value |
|---|---|
| seed | 42 |
| reps | 10 |
| prefill_orders | 10000 |
| warmup_calls | 100000 |
| measured_calls | 1000000 |
| levels_each_side | 500 |
| target_resting_orders | 10000 |
| mean_resting_orders | 6376.1 |
| min_resting_orders | 6136 |
| max_resting_orders | 6625 |
| mean_bid_levels | 337.4 |
| mean_ask_levels | 340.2 |
| final_book_size | 6436 |
| clock_resolution_ns | 41 |
| clock_overhead_p50_ns | 0 |
| clock_overhead_p99_ns | 42 |
| generation_seconds | 0.89 |
| calls_add | 549214 |
| trades_add | 0 |
| calls_cancel | 350544 |
| trades_cancel | 0 |
| calls_match_gtc | 25089 |
| trades_match_gtc | 74246 |
| calls_match_gfd | 0 |
| trades_match_gfd | 0 |
| calls_match_fak | 24989 |
| trades_match_fak | 73898 |
| calls_match_fok | 25163 |
| trades_match_fok | 74779 |
| calls_match_market | 25001 |
| trades_match_market | 74035 |
| calls_reject_fak | 0 |
| trades_reject_fak | 0 |
| calls_reject_fok | 0 |
| trades_reject_fok | 0 |
| calls_reject_market | 0 |
| trades_reject_market | 0 |
