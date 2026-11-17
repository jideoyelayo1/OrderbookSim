# OrderbookSim

## Overview

OrderbookSim is a C++ application that simulates an order book used in financial markets. It efficiently manages and matches buy and sell orders, and calculates the volume-weighted average price (VWAP). The project also includes a small online neural network, written from scratch, that learns the trade price from each trade, a WebAssembly build with a JavaScript binding, and GUIs for adding and viewing the order book in Python. This simulation helps users understand how order books work and how neural networks can be applied to financial data.

## Building and testing
Requires CMake 3.25 or later and a C++20 compiler. From the repository root:

```bash
cmake -S OrderbookSim -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j 8
ctest --test-dir build
```

This builds the order book as a static library, the `OrderbookSimExec` simulation, the
GoogleTest suite (`OrderbookTests`; GoogleTest 1.14.0 is downloaded at configure time) and the
`orderbook_bench` benchmark. The Python module is built only when the `pybind11` submodule is
checked out. Use a build directory outside `OrderbookSim/`: `OrderbookSim/build/` holds an
older Windows build of the Python module and its CMake cache.

To run the tests under AddressSanitizer and UndefinedBehaviorSanitizer:

```bash
cmake -S OrderbookSim -B build-asan -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer"
cmake --build build-asan -j 8 --target OrderbookTests && ctest --test-dir build-asan
```

The Visual Studio project files have not been updated since the headers were renamed to `.hpp`.

### WebAssembly build
Needs [emsdk](https://github.com/emscripten-core/emsdk) with Emscripten 6.0.13 (the version CI
uses) activated in the shell. CI checks out emsdk at commit
`35659964bc2762fffec0d257997fb61475f765b7`, its tag `6.0.13`, and checks `git rev-parse HEAD`
before installing. From the repository root:

```bash
source path/to/emsdk/emsdk_env.sh
web/build.sh                 # writes web/dist/orderbook.mjs and web/dist/orderbook.wasm
cmake --build build --target orderbook_parity
node web/test.mjs            # parity test against the native build
```

`web/build.sh` calls `em++` on the nine sources the order book needs plus `web/orderbook_wasm.cpp`,
with `-O2 -fexceptions -DORDERBOOK_SINGLE_THREADED`, as an ES module (`-sMODULARIZE
-sEXPORT_ES6`) for browsers and Node, with no filesystem. With Emscripten 6.0.13 the output is
56,065 bytes of JavaScript and 401,753 bytes of WebAssembly (13,803 and 155,507 gzipped at
level 9). `web/dist/` is not committed: a `v*` tag makes CI attach `orderbook.mjs`,
`orderbook.wasm` and `SHA256SUMS` to a GitHub Release.

`ORDERBOOK_SINGLE_THREADED` stops the order book from starting its Good-For-Day pruning thread,
which a single-threaded WebAssembly module cannot run; Good-For-Day orders are then cancelled only
by `EndOfDay()`. Without the flag the thread starts with the first Good-For-Day order.

`web/test.mjs` replays a seeded 20,000-call flow through the WebAssembly module and through the
native `orderbook_parity` driver and requires identical ids, statuses, trades, levels, sizes and
VWAP. CMake compiles the driver from the sources with `ORDERBOOK_SINGLE_THREADED`, as
`web/build.sh` compiles the module, so neither side has the 16:00 pruning thread. The network's prediction and error start from `rand()` weights and see the wall clock, so
they are only required to be finite. Because both sides share `Book.hpp`, the test also pins the
flow's trade count and its count of each outcome, and checks malformed JavaScript calls on the
WebAssembly side (wrong types, missing arguments, values over the limits). It uses Node built-ins
only.

### JavaScript binding
`web/orderbook_wasm.cpp` binds `Book` (`web/Book.hpp`), a wrapper that assigns its own order ids
(1, 2, 3, ... per valid add; never reused) and checks every argument:

```js
import createModule from './orderbook.mjs';

const { Book } = await createModule();
const book = new Book();
book.add('GoodTillCancel', 'Sell', 100, 5); // { id: 1, status: 'resting', restingQuantity: 5, trades: [] }
book.add('FillAndKill', 'Buy', 101, 8);     // { id: 2, status: 'partial', restingQuantity: 0, trades: [{ bidId: 2, bidPrice: 101, askId: 1, askPrice: 100, quantity: 5 }] }
book.delete();                              // frees the C++ object
```

| method | returns |
|---|---|
| `add(type, side, price, qty)` | `{ id, status, restingQuantity, trades }`. `type` is `GoodTillCancel`, `FillAndKill`, `Market`, `GoodForDay` or `FillOrKill`; `side` is `Buy` or `Sell`; `price` and `qty` are whole numbers from 1 to 1,000,000 (`price` is ignored for `Market`) |
| `cancel(id)` | `true` if the order was resting and is now cancelled |
| `modify(id, side, price, qty)` | as `add`; the order keeps its id and type and loses time priority |
| `endOfDay()` | nothing; cancels every Good-For-Day order |
| `levels()` | `{ bids: [{ price, quantity }], asks: [...] }`, best price first |
| `size()` | number of resting orders |
| `prediction()` | the network's prediction for the latest trade, made before training on it (0 before any trade) |
| `recentError()` | the network's smoothed RMS error, in units of price / 500 |
| `vwap()` | volume-weighted average trade price, truncated to a whole price (0 before any trade) |
| `reset()` | nothing; replaces the book and the network with new ones and restarts ids at 1 |

`status` is `filled` (all of the quantity traded), `partial` (some traded), `resting` (none
traded, the order rests), `rejected` (none traded, nothing rests) or `invalid` (bad arguments:
the book is untouched, no id is used and `id` is 0). `restingQuantity` is how much of the order is
left in the book. Each trade names the bid and the ask by id and limit price; the order book
records the trade at the bid's price, which is the price VWAP and the network use. By order type:

| type | outcomes |
|---|---|
| `GoodTillCancel`, `GoodForDay` | `resting`, `partial` with the remainder resting, `filled` |
| `FillAndKill` | `rejected` if nothing crosses, `partial` with `restingQuantity` 0 (the remainder is cancelled), `filled` |
| `FillOrKill` | `rejected` unless the whole quantity can trade, otherwise `filled` |
| `Market` | `rejected` if the opposite side is empty, `filled`, or `partial` with the remainder resting at the worst opposite price |
| `modify` | `rejected` if the id is not resting, otherwise as for the order's type |

Ids cross as JavaScript numbers (exact below 2^53), not BigInt. A C++ exception, which the order
book does not throw in normal use, reaches JavaScript as a `CppException`. `Book` is not
thread-safe: use each one from one thread at a time (the WebAssembly module has only one).

Number arguments must be JavaScript numbers and string arguments JavaScript strings. Anything
else (`'5'`, `true`, `null`, `undefined`, a missing argument, a BigInt) makes `add` and `modify`
return `invalid` and `cancel` return `false`; it is not coerced, and no method throws for bad
arguments.

Prices and quantities above 1,000,000 are `invalid` (`Book::maxPrice` and `Book::maxQuantity`).
The cap keeps one trade's price × quantity at or below 10^12, so the 64-bit VWAP sums stay exact
for 9,223,372 trades even at the maximum price and quantity, and the quantity the network receives
stays far inside an `int`. A level's total quantity is a 64-bit sum, so it does not wrap
however many orders rest at one price.

## Running the program
In order to run the otherbook stimulation navigate to build 

```bash
python .\OrderbookView.py # to view orders
python .\addOrder.py
```

## Key Components

### Orderbook
The main class representing the order book. It manages buy and sell orders, matches them based on price-time priority, maintains an order history, and calculates VWAP.

### Order
Represents an individual order with attributes such as order type, side (buy/sell), price, and quantity.

### Order Types
The project supports various order types including:
- **Good Till Cancel**: The order remains active until filled, canceled, or it expires.
- **Fill and Kill**: The order is filled as much as possible, and any remaining quantity is canceled immediately.
- **Market Order**: Filled at the best available price.
- **Good For Day**: Similar to Good Till Cancel, but cancelled by `EndOfDay()` and, except in `ORDERBOOK_SINGLE_THREADED` builds, by a thread at 16:00 local time that starts with the first Good-For-Day order.
- **Fill or Kill**: Must be filled immediately and entirely or canceled.

### Order Modify
Handles modifications to existing orders, such as price or quantity adjustments.

### Trade
Represents a trade executed between a buy and sell order.

### VWAP (Volume-Weighted Average Price)
Calculates the average price of orders based on their volume.

### Neural Network for Price Prediction
A small online model written from scratch: a fully connected network with two inputs, one
hidden layer of three tanh neurons and one output, trained by one step of backpropagation on
every trade. Its two inputs are the local time of day (in whole hundredths of a day) and the traded
quantity; its target is the trade price; all three are divided by `PRICESCALE` (500 by default).
Because the output is a tanh, predictions stay within ±`PRICESCALE`. The weights start from
`rand()`. `Orderbook` exposes the prediction for the latest trade (made before training on it),
the network's recent average error and the VWAP.

## GUI Integration

### Python Bindings
OrderbookSim now includes Python bindings using Pybind11, allowing for integration with Python GUIs.

```cpp
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "Orderbook.h"

namespace py = pybind11;

PYBIND11_MODULE(orderbook, handle) {
    handle.doc() = "Orderbook"; // optional module docstring
    py::class_<Orderbook>(handle, "Orderbook")
        .def(py::init<>())
        .def("addOrder", &Orderbook::addOrderViaPython)
        .def("loadToJson", &Orderbook::saveToJson);
}
```

### GUI for Viewing Orderbook
A Python GUI to view the order book using Tkinter:

```python
import json
import tkinter as tk
from tkinter import ttk

def load_json(filename):
    with open(filename, 'r') as file:
        return json.load(file)

def create_table(frame, headers, data):
    tree = ttk.Treeview(frame, columns=headers, show='headings')
    for header in headers:
        tree.heading(header, text=header)
        tree.column(header, width=100)

    for row in data:
        tree.insert('', 'end', values=[row.get(header, '') for header in headers])

    tree.pack(expand=True, fill='both')
    return tree

def refresh_data():
    global sell_orders, buy_orders, purchases, live_sell_orders, live_buy_orders

    sell_orders = load_json('SellHistory.json')
    buy_orders = load_json('BuyHistory.json')
    purchases = load_json('PurchaseHistory.json')
    live_sell_orders = load_json('SellLiveHistory.json')
    live_buy_orders = load_json('BuyLiveHistory.json')

    update_table(frame_sell_orders, sell_order_headers, sell_orders)
    update_table(frame_buy_orders, buy_order_headers, buy_orders)
    update_table(frame_purchases, purchase_headers, purchases)
    update_table(frame_live_sell_orders, live_sell_order_headers, live_sell_orders)
    update_table(frame_live_buy_orders, live_buy_order_headers, live_buy_orders)

    root.after(10000, refresh_data)  # Schedule the next refresh in 10 seconds

def update_table(frame, headers, data):
    for widget in frame.winfo_children():
        widget.destroy()
    create_table(frame, headers, data)

root = tk.Tk()
root.title("Order Book")
root.geometry("1000x600")

notebook = ttk.Notebook(root)
notebook.pack(expand=True, fill="both")

frame_sell_orders = tk.Frame(notebook)
frame_buy_orders = tk.Frame(notebook)
frame_purchases = tk.Frame(notebook)
frame_live_sell_orders = tk.Frame(notebook)
frame_live_buy_orders = tk.Frame(notebook)

notebook.add(frame_sell_orders, text="Sell Orders")
notebook.add(frame_buy_orders, text="Buy Orders")
notebook.add(frame_purchases, text="Purchases")
notebook.add(frame_live_sell_orders, text="Live Sell Orders")
notebook.add(frame_live_buy_orders, text="Live Buy Orders")

sell_order_headers = ["order_id", "order_type", "price", "quantity", "side", "time"]
buy_order_headers = ["order_id", "order_type", "price", "quantity", "side", "time"]
purchase_headers = ["price", "quantity", "time"]
live_sell_order_headers = ["order_id", "order_type", "price", "quantity", "side", "time"]
live_buy_order_headers = ["order_id", "order_type", "price", "quantity", "side", "time"]

refresh_data()

root.mainloop()
```

### GUI for Adding Orders
A Python GUI to add orders to the order book using Tkinter:

```python
import tkinter as tk
from tkinter import messagebox
import orderbook  # Made via pybind

class OrderEntryGUI:
    def __init__(self, master):
        self.master = master
        self.orderbook = orderbook.Orderbook()

        self.order_id_counter = 0

        self.order_type_mapping = {
            'GoodTillCancel': 1,
            'FillAndKill': 2,
            'Market': 3,
            'GoodForDay': 4,
            'FillOrKill': 5
        }

        self.type_var = tk.StringVar(master)
        self.side_var = tk.StringVar(master)
        self.price_entry = tk.Entry(master)
        self.qty_entry = tk.Entry(master)

        self.create_widgets()

    def create_widgets(self):
        tk.Label(self.master, text="Type").grid(row=0, column=0)
        tk.Label(self.master, text="Side").grid(row=1, column=0)
        tk.Label(self.master, text="Price").grid(row=2, column=0)
        tk.Label(self.master, text="Quantity").grid(row=3, column=0)

        types = ['GoodTillCancel', 'FillAndKill', 'Market', 'GoodForDay', 'FillOrKill']
        tk.OptionMenu(self.master, self.type_var, *types).grid(row=0, column=1)
        sides = ['Buy', 'Sell']
        tk.OptionMenu(self.master, self.side_var, *sides).grid(row=1, column=1)
        self.price_entry.grid(row=2, column=1)
        self.qty_entry.grid(row=3, column=1)

        tk.Button(self.master, text="Add Order", command=self.add_order).grid(row=4, column=0, columnspan=2)
        tk.Button(self.master, text="Reset", command=self.reset).grid(row=5, column=0, columnspan=2)

    def add_order(self):
        order_type_str = self.type_var.get()
        order_type = self.order_type_mapping.get(order_type_str)
        side = self.side_var.get()

        try:
            price = int(self.price_entry.get())
            qty = int(self.qty_entry.get())
        except ValueError:
            messagebox.showerror("Error", "Price and quantity must be numbers.")
            return

        order_id = self.order_id_counter
        self.order_id_counter += 1

        self.orderbook.addOrder(order_type, order_id, 1 if side == 'Buy' else 2, price, qty)
        print("Order added:", order_type_str, order_id, side, price, qty)

        messagebox.showinfo("Order Added", "Order added successfully.")

        self.type_var.set('')
        self.side_var.set('')
        self.price_entry.delete(0, tk.END)
        self.qty_entry.delete(0, tk.END)

    def reset(self):
        self.type_var.set('')
        self.side_var.set('')
        self.price_entry.delete(0, tk.END)
        self.qty_entry.delete(0, tk.END)

if __name__ == "__main__":
    root = tk.Tk()
    root.title("Order Entry")
    app = OrderEntryGUI(root)
    root.mainloop()
```

## Usage

### Example
Here's a simple example of how to use the order book:

```cpp
#include "Orderbook.hpp"

int main() {
    Orderbook orderbook;

    const OrderId orderId = 1;
    orderbook.addOrder(std::make_shared<Order>(OrderType::GoodTillCancel, orderId, Side::Buy, 100, 10));

    std::cout << "Orderbook size: " << orderbook.Size() << std::endl;

    orderbook.CancelOrder(orderId);

    std::cout << "Orderbook size after cancellation: " << orderbook.Size() << std::endl;

    return 0;
}
```

### Simulation
The `OrderbookSim.cpp` file contains the main function and a simulation function `randomOrderSimulation` to generate random orders:

```cpp
#include <iostream>
#include "Orderbook.hpp"
#include <random>

void randomOrderSimulation(const int n);

int main() {
    const int n = 1000;
    randomOrderSimulation(n);
    return 0;


}

void randomOrderSimulation(const int n) {
    Orderbook orderbook;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> type(1, 5);
    std::uniform_int_distribution<> side(1, 100);
    std::uniform_int_distribution<> price(1, 500);
    std::uniform_int_distribution<> quantity(1, 500);

    for (int i = 0; i < n; i++) {
        orderbook.addOrder(intToOrdertType(type(gen)), i, side(gen) == 1 ? Side::Buy : Side::Sell, price(gen), quantity(gen));
    }

    orderbook.printAllOrders();
    std::cout << "OrderBook Size: " << orderbook.Size() << std::endl;
}
```

### Neural Network Price Prediction
The network trains itself as orders match; there is nothing to configure. Read it with
`orderbook.getPrediction()` and `orderbook.getRecentAverageError()`, and the VWAP with
`orderbook.getVWAP()`.

## Dependencies
The project relies on standard C++ libraries, nlohmann/json (vendored as `json.hpp`), GoogleTest
for the tests (fetched by CMake) and, for the Python module only, pybind11 (submodule).

## Testing
`ctest --test-dir build` runs 59 GoogleTest cases:

- `OrderbookTest/test.cpp`: seven data-driven cases from `OrderbookTest/TestFiles/`; they check
  only order and level counts.
- `OrderbookTest/MatchingContractTests.cpp`: price priority on both sides, FIFO within a level,
  no trade without a cross, each order type (Good-Till-Cancel, Fill-and-Kill, Fill-or-Kill,
  Market, Good-For-Day with `EndOfDay`), cancel, modify, a seeded 20,000-call random flow
  checked after every call, and destroying many books that each started a pruning thread.
- `OrderbookTest/HistoryTests.cpp`: the network's time-of-day input and its training, and the
  `hasOrder`, prediction, error and VWAP read-outs.
- `OrderbookTest/BookTests.cpp`: the JavaScript binding's `Book` run natively: the statuses of
  each order type, ids, reset, argument checks and the price and quantity limits.

With the bid/ask ordering fix of PR #6 reverted, 16 of the 37 cases in the first two files fail
while all seven data-driven cases still pass. The suite is behavioural: it does not cover
concurrent callers or the 16:00 Good-For-Day pruning thread. Logs are in
`OrderbookSim/bench/results/`. The WebAssembly parity test is described under
[WebAssembly build](#webassembly-build).

## Performance
Measured with `OrderbookSim/bench/run.sh --reps 10 --seed 42` on an Apple M1 Pro (macOS 27.0.1,
Apple clang 21, `-O3`), 1,000,000 measured calls per repetition with about 640 resting orders
over about 38 price levels per side. Median over 10 repetitions:

| call | p50 | p99 | p99.9 | allocations per call |
|---|---:|---:|---:|---:|
| add (rests) | 208 ns | 625 ns | 833 ns | 5.49 |
| cancel | 125 ns | 333 ns | 459 ns | 0 |
| match (about 3 trades; any order type) | 1,667–1,708 ns | 4,062–4,125 ns | 5,333–5,438 ns | 20.8 |

Throughput of the mixed flow: 2.60 million calls per second. The timer resolution is 41.67 ns,
the machine was not isolated, and the slowest add in each repetition (11.5–18.6 ms) is a rehash of
the history's live-order map, which keeps cancelled orders. Most of a match's time is history and
neural-network bookkeeping rather than the `std::map` price levels. Method, workload, raw data
and caveats: [`OrderbookSim/bench/RESULTS.md`](OrderbookSim/bench/RESULTS.md).

## Contributing
Contributions are welcome! Feel free to open issues or pull requests for any improvements or bug fixes.

## License
This project is licensed under the MIT License - see the LICENSE file for details.

## Contact
For any inquiries or feedback, please contact [JideOyelayo1@Gmail.com].
