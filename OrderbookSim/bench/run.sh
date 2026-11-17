#!/bin/sh
# Build Release, record the environment, run the benchmark and write a Markdown summary.
#   OrderbookSim/bench/run.sh [orderbook_bench options]     e.g. --reps 10 --seed 42
# Results go to OrderbookSim/bench/results (override with OUT=dir).
set -eu
here=$(cd "$(dirname "$0")" && pwd)
src=$(dirname "$here")
root=$(dirname "$src")
build="$root/build"
out="${OUT:-$here/results}"

cmake -S "$src" -B "$build" -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON > /dev/null
cmake --build "$build" --target orderbook_bench -j 8 > /dev/null
mkdir -p "$out"

{
    echo "start_utc: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
    echo "local_time: $(date +%H:%M) (the Good-For-Day pruning thread acts at 16:00 local)"
    echo "git_revision: $(git -C "$root" rev-parse HEAD)$(git -C "$root" diff --quiet HEAD -- "$src" || echo ' (uncommitted changes)')"
    echo "os: $(uname -srm)"
    if command -v sw_vers > /dev/null; then echo "macos: $(sw_vers -productVersion) ($(sw_vers -buildVersion))"; fi
    echo "cpu: $(sysctl -n machdep.cpu.brand_string 2> /dev/null || grep -m1 'model name' /proc/cpuinfo)"
    echo "logical_cpus: $(sysctl -n hw.ncpu 2> /dev/null || nproc)"
    echo "memory_bytes: $(sysctl -n hw.memsize 2> /dev/null || grep -m1 MemTotal /proc/meminfo)"
    echo "compiler: $(grep -m1 'CMAKE_CXX_COMPILER:' "$build/CMakeCache.txt" | cut -d= -f2) ($("$(grep -m1 'CMAKE_CXX_COMPILER:' "$build/CMakeCache.txt" | cut -d= -f2)" --version | head -1))"
    echo "compile_command_Orderbook.cpp: $(python3 -c 'import json,sys; print(next(c["command"] for c in json.load(open(sys.argv[1])) if c["file"].endswith("/Orderbook.cpp")))' "$build/compile_commands.json")"
    echo "load_average_before: $(uptime | sed 's/.*load averages*: //')"
    if command -v pmset > /dev/null; then echo "power: $(pmset -g batt | head -1)"; fi
    echo "bench_options: $*"
} | sed "s#$root#<repo>#g" > "$out/environment.txt"

"$build/orderbook_bench" --out "$out" "$@"
{ echo "end_utc: $(date -u +%Y-%m-%dT%H:%M:%SZ)"; echo "load_average_after: $(uptime | sed 's/.*load averages*: //')"; } >> "$out/environment.txt"
python3 "$here/summarise.py" "$out" > "$out/summary.md"
cat "$out/summary.md"
