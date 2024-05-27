import tkinter as tk
from tkinter import messagebox
import orderbook ## Made via pybind

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

        # update Order Id
        order_id = self.order_id_counter
        self.order_id_counter += 1

        # Add the order to the order book
        self.orderbook.addOrder(order_type, order_id, 1 if side == 'Buy' else 2, price, qty)
        print("Order added:", order_type_str, order_id, side, price, qty)

        # Show confirmation message
        messagebox.showinfo("Order Added", "Order added successfully.")

        # Reset fields after successful order
        self.type_var.set('')
        self.side_var.set('')
        self.price_entry.delete(0, tk.END)
        self.qty_entry.delete(0, tk.END)

    def reset(self):
        # Reset all fields
        self.type_var.set('')
        self.side_var.set('')
        self.price_entry.delete(0, tk.END)
        self.qty_entry.delete(0, tk.END)

if __name__ == "__main__":
    root = tk.Tk()
    root.title("Order Entry")
    app = OrderEntryGUI(root)
    root.mainloop()
