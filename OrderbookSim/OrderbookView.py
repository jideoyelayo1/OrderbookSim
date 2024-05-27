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
