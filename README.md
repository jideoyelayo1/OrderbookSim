# OrderbookSim

## Overview

OrderbookSim is a C++ application that simulates an order book used in financial markets. It efficiently manages and matches buy and sell orders, and calculates the volume-weighted average price (VWAP). The project also includes a neural network built from scratch to predict sell prices, and now features GUIs for adding and viewing the order book in Python. This simulation helps users understand how order books work and how neural networks can be applied to financial data.

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
- **Good For Day**: Similar to Good Till Cancel, but automatically canceled at the end of the trading day.
- **Fill or Kill**: Must be filled immediately and entirely or canceled.

### Order Modify
Handles modifications to existing orders, such as price or quantity adjustments.

### Trade
Represents a trade executed between a buy and sell order.

### VWAP (Volume-Weighted Average Price)
Calculates the average price of orders based on their volume.

### Neural Network for Price Prediction
A custom-built neural network designed to predict sell prices based on historical order data. This neural network provides a foundation for applying machine learning to financial market simulations.

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
#include "Orderbook.h"

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
#include "Orderbook.h"
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
        orderbook.addOrder(intToOrderType(type(gen)), i, side(gen) == 1 ? Side::Buy : Side::Sell, price(gen), quantity(gen));
    }

    orderbook.printAllOrders();
    std::cout << "OrderBook Size: " << orderbook.Size() << std::endl;
}
```

### Neural Network Price Prediction
To use the neural network for predicting sell prices, you need to provide training data and configure the network parameters. The neural network implementation is integrated into the project to enhance the simulation's predictive capabilities.

## Dependencies
The project relies on standard C++ libraries and includes the Google Test framework for testing purposes.

## Testing
The project includes comprehensive unit tests implemented using the Google Test framework to ensure correctness and reliability. To run the tests, use the following file:

- `test.cpp`

## Contributing
Contributions are welcome! Feel free to open issues or pull requests for any improvements or bug fixes.

## License
This project is licensed under the MIT License - see the LICENSE file for details.

## Contact
For any inquiries or feedback, please contact [JideOyelayo1@Gmail.com].
