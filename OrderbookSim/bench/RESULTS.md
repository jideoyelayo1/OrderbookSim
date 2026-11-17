# Benchmark and test results

Question: what does supporting several order types cost on the matching path of the
`std::map` baseline, and does the book keep price-time priority?

The book measured is `Orderbook` as it is in this repository: per side a
`std::map<Price, std::list<OrderPtr>>` (bids `std::greater`, asks `std::less`), an
`unordered_map` from order id to order, and an `unordered_map` of per-price level data. The
container was not changed. Every `addOrder` also writes to `OrderDetailHistory` (buy, sell and
purchase history, a live-order map, and one neural-network training step per trade), because
that is what `addOrder` does here; the numbers are for that path, not for a bare matching
engine.

All raw output is in [`results/`](results/). Nothing below is estimated.

## Reproduce

From the repository root:

```sh
cmake -S OrderbookSim -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j 8
ctest --test-dir build                                    # 37 tests
OrderbookSim/bench/run.sh --reps 10 --seed 42             # benchmark, writes OrderbookSim/bench/results
```

The runs reported here:

| results directory | command |
|---|---|
| `results/main` | `OUT=$PWD/OrderbookSim/bench/results/main OrderbookSim/bench/run.sh --reps 10 --seed 42 --levels 50 --target 1000` |
| `results/seed7` | same with `--seed 7` |
| `results/levels10` | same with `--seed 42 --levels 10 --target 200` |
| `results/levels500` | same with `--seed 42 --levels 500 --target 10000` |

`run.sh` builds Release, writes `environment.txt`, runs `orderbook_bench` and writes
`summary.md` with `summarise.py` (standard-library Python). Files per run:
`latency_per_rep.csv` (one row per repetition and operation), `throughput_per_rep.csv`,
`latency_histogram.csv` (pooled over repetitions, 20 log-spaced bins per decade),
`workload.csv`, `memory.csv`.

## Environment (from `results/main/environment.txt`)

| | |
|---|---|
| Machine | Apple M1 Pro, 10 logical CPUs, 16 GiB, on AC power |
| OS | macOS 27.0.1 (26A434), Darwin 27.0.0 arm64 |
| Compiler | Apple clang 21.0.0 (clang-2100.3.34.2), libc++ |
| Flags | `-O3 -DNDEBUG -std=gnu++20 -arch arm64` (CMake Release) |
| Source revision | `432ef48` (book code unchanged since `d3c7f24`) |
| Run | 2026-11-16 22:06:27 to 22:06:38 UTC, local time 22:06 |
| Other load | 1-minute load average 23.65 before, 21.14 after: other work was running on the machine; it was not isolated |

## Workload

Seeded `std::mt19937_64` (seed 42) with `std::uniform_*` distributions. Those distributions
are implementation-defined, so the same seed gives the same flow with libc++ but a different
one with another standard library.

- Prices are integer ticks around a fixed mid of 100,000. Passive orders are placed uniformly
  at mid ± [1, L] ticks, L = 50.
- Prefill: 1,000 passive orders. Warm-up: 100,000 calls, replayed untimed before every timed
  pass. Measured: 1,000,000 calls per repetition.
- Each call: 10% aggressive (Market, Fill-and-Kill, Fill-or-Kill or Good-Till-Cancel, equally
  likely; buy or sell equally likely; limit L ticks through the mid; quantity uniform 1–200);
  otherwise a passive add with probability 1000 / (1000 + resting orders), else a cancel of a
  uniformly chosen resting order. Passive orders are 90% Good-Till-Cancel and 10% Good-For-Day,
  quantity uniform 1–100. There are no modifies in the mix.
- Calls are classified by outcome: `add` rested without trading, `match_*` traded at least once
  (by order type), `reject_*` was an aggressive order that did not trade, `cancel`.

Realised (from `results/main/workload.csv`): 549,002 adds (54.9%), 350,996 cancels (35.1%),
100,002 matches (25,059 Good-Till-Cancel, 24,821 Fill-and-Kill, 25,147 Fill-or-Kill, 24,975
Market), 0 rejects; 2.95–2.96 trades per match. Resting orders: mean 640, min 537, max 772.
Price levels: mean 37.9 bid and 38.0 ask.

## Controls

- Timer: `std::chrono::steady_clock`. Measured resolution on this machine is 41.67 ns
  (reported as 41 or 42 ns); back-to-back reads take 0 ns at p50 (same tick) and 42 ns at p99.
  Per-call latencies are therefore quantised in ~42 ns steps and are not overhead-corrected.
- Allocations: the benchmark binary replaces global `operator new`/`delete` and counts calls and
  requested bytes inside each timed call. Allocations made with C `malloc` inside libc (for
  example in `localtime_r`) are not counted.
- Pinning: macOS has no thread-affinity API. The thread runs at `QOS_CLASS_USER_INTERACTIVE`
  so the scheduler prefers performance cores; which core ran was not verified.
- Repetitions: 10, each on a fresh book, replaying the identical call sequence. Every replayed
  call is checked to produce the same number of trades as the generation pass.
- Throughput is a separate pass per repetition: the measured block timed as a whole, no per-call
  timer.
- Not controlled: other processes, CPU frequency and thermal state.

## Results: main configuration

Latency in ns. Each cell is the median over 10 repetitions, with the IQR across repetitions in
brackets. "IQR within run" is p75 − p25 of the per-call distribution (median over
repetitions), so "p50 ± IQR within run" is the per-call median ± IQR.

| op | calls per rep | trades per call | p50 | IQR within run | p90 | p99 | p99.9 | max (median of reps) | max (all reps) | allocations per call | bytes requested per call |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| add | 549,002 | 0.00 | 208 [208–208] | 125 [84–125] | 396 [375–417] | 625 [594–625] | 833 [792–875] | 13,832,312 [13,047,031–15,717,542] | 18,559,583 | 5.49 | 20,867 |
| cancel | 350,996 | 0.00 | 125 [125–125] | 82 [82–82] | 208 [208–208] | 333 [292–333] | 459 [458–563] | 20,437 [13,938–49,760] | 86,958 | 0.00 | 0 |
| match, Good-Till-Cancel | 25,059 | 2.96 | 1,667 [1,667–1,667] | 1,167 [1,166–1,198] | 2,916 [2,875–2,917] | 4,062 [4,042–4,125] | 5,333 [5,250–6,167] | 38,271 [20,396–56,031] | 157,083 | 20.82 | 20,906 |
| match, Fill-and-Kill | 24,821 | 2.96 | 1,667 [1,667–1,667] | 1,167 [1,167–1,198] | 2,917 [2,916–2,958] | 4,084 [4,042–4,208] | 5,354 [5,292–7,177] | 60,479 [52,978–147,938] | 196,958 | 20.82 | 21,040 |
| match, Fill-or-Kill | 25,147 | 2.96 | 1,708 [1,667–1,708] | 1,167 [1,167–1,167] | 2,958 [2,917–2,959] | 4,125 [4,125–4,240] | 5,396 [5,240–6,052] | 50,833 [20,969–76,167] | 104,000 | 20.81 | 20,916 |
| match, Market | 24,975 | 2.95 | 1,667 [1,666–1,667] | 1,167 [1,166–1,198] | 2,916 [2,875–2,917] | 4,063 [4,041–4,198] | 5,438 [5,239–5,979] | 163,916 [136,938–173,427] | 572,375 | 20.76 | 21,419 |

Throughput of the mixed flow: **2,597,382 calls/s** median [IQR 2,547,452–2,627,365] over 10
repetitions. Peak resident set size of the benchmark process: 170 MiB (this includes the
benchmark's own buffers, about 64 MB for the recorded calls and samples).

## Sensitivity: seed and book depth

Median over 10 repetitions [IQR across repetitions]; latency in ns.

| run | resting orders (mean) | levels per side (mean) | add p50 / p99 | cancel p50 / p99 | match GTC p50 / p99 | match FOK p50 / p99 | bytes per add | calls/s |
|---|---:|---:|---|---|---|---|---:|---|
| `levels10` (L = 10) | 130 | 8.6 | 167 / 520 | 83 / 167 | 1,542 / 3,687 | 1,542 / 3,771 | 4,521 | 3,091,216 [3,058,724–3,133,428] |
| `main` (L = 50) | 640 | 37.9 / 38.0 | 208 / 625 | 125 / 333 | 1,667 / 4,062 | 1,708 / 4,125 | 20,867 | 2,597,382 [2,547,452–2,627,365] |
| `seed7` (L = 50) | 640 | 37.9 / 38.1 | 208 / 584 | 125 / 292 | 1,666 / 4,041 | 1,688 / 4,104 | 20,839 | 2,731,103 [2,672,740–2,757,988] |
| `levels500` (L = 500) | 6,376 | 337.4 / 340.2 | 375 / 833 | 292 / 792 | 1,958 / 4,958 | 2,000 / 5,146 | 204,403 | 1,791,210 [1,783,234–1,826,432] |

Seed 7 against seed 42 moves throughput by about 5% and p50s by at most one timer tick; the
machine was shared, so part of that is run-to-run noise.

## Where the time goes

`results/profile/`: `sample` (macOS, 1 ms interval, 8 s) on the main configuration, summarised
by `profile_summary.py` as inclusive samples. Book calls are `addOrder` plus `CancelOrder`
(3,353 of 4,084 main-thread samples; the rest is the benchmark itself).

| inclusive frame | share of book calls |
|---|---:|
| `Orderbook::MatchOrders()` | 45.5% |
| `OrderDetailHistory::updateNeutralNetwork` (one per trade) | 26.8% |
| of which `localtime_r` (all callers) | 9.3% |
| of which `NeuralNetwork::Run` | 4.4% |
| `OrderDetailHistory::addOrderToHistory` (every add) | 18.8% |
| `std::__hash_table` (all `unordered_map` work) | 30.2% |
| `std::__tree` (all `std::map` work) | 1.2% |
| `operator new` / `free` | 16.1% / 5.9% |

Findings:

1. **Order types cost little on the matching path.** Matches by Good-Till-Cancel, Fill-and-Kill,
   Fill-or-Kill and Market have the same p50 to within one timer tick (1,667–1,708 ns in
   `main`). Fill-or-Kill's pre-check, which walks the whole level-data map for both sides, adds
   one tick at p50; at p99 it adds 41–63 ns in `main` and 104–188 ns at 340 levels per side
   (`levels500`).
2. **`std::map` is not the limiter.** Red-black-tree work is 1.2% of book time. Of the 45.5%
   spent in `MatchOrders`, 32.8 points are history and neural-network bookkeeping called from
   it (`updateNeutralNetwork` 26.8, `removeMatchedOrder` 3.5, `addOrderToPurchaseHistory` 2.5).
   The neural-network update includes `localtime_r` for a time-of-day input that
   `static_cast<int>(fraction) * 100` always truncates to 0. Hash-map operations on all paths
   are 30.2%.
3. **Allocations.** 5.49 per add, about 20.8 per match (about 7 per trade), 0 per cancel.
   About 20.9 KB is requested on every add and match because `MatchOrders` reserves room for
   `_orders.size()` trades (32 bytes each) on every call, traded or not; that cost scales with
   resting orders (4.5 KB at 130 resting orders, 204 KB at 6,376).
4. **The maximum is history growth.** In all 40 repetitions of the four runs the slowest add
   (8.6–18.7 ms) requested 3.30–3.50 MB, of which 3,292,216 bytes is a rehash of the live-order
   `unordered_map` to 411,527 buckets and the rest the per-call trade reservation
   (`slowest_call_bytes` in `latency_per_rep.csv`; sizes identified in
   `results/large_allocations.txt`). Cancelled orders never leave
   that map, so it grows with every order ever added. In `main` the slowest Market match in 9 of
   10 repetitions requested 8,409,436 bytes: the purchase-history vector doubling to 2^19
   entries.
5. **Depth.** From 8.6 to 38 to 340 levels per side, add p50 goes 167 → 208 → 375 ns, cancel
   83 → 125 → 292 ns, match 1.54 → 1.67 → 1.96 µs and throughput 3.09 → 2.60 → 1.79 M calls/s.
   These runs do not separate `std::map` depth from the per-call reservation in finding 3 and the
   larger working set of the hash maps.

## Correctness tests

`ctest --test-dir build` runs 37 tests: the 7 original data-driven cases (`test.cpp`,
`TestFiles/`) and 30 in `OrderbookTest/MatchingContractTests.cpp` covering price priority on
both sides, FIFO within a level, crossing, each order type, cancel, modify, a 20,000-call
seeded random flow checked after every call, and book destruction.

| log | what it shows |
|---|---|
| `results/tests_before_fixes.txt` | at `96ac420` (tests added, no fixes; 36 tests then): 6 failures. Two assertion failures from `EndOfDay`; Fill-and-Kill partial fill and the random flow time out (self-deadlock); two more tests time out at random (destructor lost wake-up), here including the original `Match_GoodTillCancel.txt` case |
| `results/asan_same_price_cross_before_fix.txt` | AddressSanitizer: heap-use-after-free in `MatchOrders` (`Orderbook.cpp:110`) at `96ac420` |
| `results/tests_with_pr6_reverted.txt` | at `d3c7f24` with only PR #6 reverted (bid/ask comparators and the crossing check back to the original): 16 of 37 fail. The 7 original data-driven cases all pass, so the original suite cannot detect the reversed ordering |
| `results/tests_after_fixes.txt` | at `d3c7f24`: 37 of 37 pass, each run 20 times (`--repeat until-fail:20`) |
| `results/tests_after_fixes_asan.txt` | at `d3c7f24`, Debug with `-fsanitize=address,undefined`: 37 of 37 pass |

## Not measured here

- Other hardware, Linux, GCC or MSVC; an isolated machine; hardware performance counters.
- Modify latency (no modifies in the benchmark mix); concurrent callers.
- Memory per resting order: only process peak RSS, which includes the benchmark's buffers.
- A latency or memory budget: the project states none, so there is nothing to compare against.
- The original `jideoyelayo1/orderbook` repository: its bid/ask ordering is reversed, so its
  timings would describe a book that matches the wrong orders.
