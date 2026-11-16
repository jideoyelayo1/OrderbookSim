"""Inclusive sample counts per frame from a macOS `sample` report, main thread only.

usage: python3 profile_summary.py SAMPLE_REPORT.txt   (standard library only)
A frame's count includes its callees; nested occurrences of the same frame are not counted twice.
Frames are matched by prefix. `mach_absolute_time` is the benchmark's own timer, outside the book.
"""
import re
import sys
from collections import defaultdict

BOOK = ["Orderbook::addOrder(OrderType", "Orderbook::CancelOrder(unsigned long long)"]
FRAMES = BOOK + [
    "Orderbook::MatchOrders()",
    "OrderDetailHistory::addOrderToHistory",
    "OrderDetailHistory::removeMatchedOrder",
    "OrderDetailHistory::addOrderToPurchaseHistory",
    "OrderDetailHistory::updateNeutralNetwork",
    "NeuralNetwork::Run",
    "localtime_r",
    "std::chrono::system_clock::now()",
    "Orderbook::canFullyFill",
    "std::__tree",
    "std::__hash_table",
    "operator new(unsigned long)",
    "_free",
    "mach_absolute_time",
]

LINE = re.compile(r"^(?P<lead>[ +!:|]*)(?P<count>\d+) (?P<name>.+?)(  \(in .*)?$")


def parse(path):
    lines = open(path, encoding="utf-8", errors="replace").read().split("\n")
    start = next(i for i, l in enumerate(lines) if l.startswith("Call graph:"))
    nodes = []
    for l in lines[start + 1:]:
        if not l.strip():
            break
        m = LINE.match(l)
        if m:
            nodes.append((len(m.group("lead")), int(m.group("count")), m.group("name").strip()))
    return nodes


def main(path):
    nodes = parse(path)
    top = min(d for d, _, _ in nodes)
    threads = [i for i, (d, _, _) in enumerate(nodes) if d == top]
    first = next(i for i in threads if "main-thread" in nodes[i][2])
    end = next((i for i in threads if i > first), len(nodes))
    total = nodes[first][1]
    inclusive = defaultdict(int)
    stack = []
    for depth, count, name in nodes[first + 1:end]:
        while stack and stack[-1][0] >= depth:
            stack.pop()
        for frame in FRAMES:
            if name.startswith(frame) and not any(n.startswith(frame) for _, n in stack):
                inclusive[frame] += count
        stack.append((depth, name))
    book = sum(inclusive[f] for f in BOOK)
    print(f"main-thread samples: {total}; inside the order book's public calls: {book}\n")
    print("| frame (inclusive; prefix match) | samples | share of book calls | share of main thread |")
    print("|---|---:|---:|---:|")
    for frame in FRAMES:
        if inclusive[frame]:
            print(f"| `{frame}` | {inclusive[frame]} | {100 * inclusive[frame] / book:.1f}% | {100 * inclusive[frame] / total:.1f}% |")


if __name__ == "__main__":
    main(sys.argv[1])
