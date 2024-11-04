import tkinter as tk
from tkinter import messagebox
import orderbook  # Made via pybind, providing access to C++ Orderbook class in Python

class OrderEntryGUI:
    """
    GUI application for adding orders to an order book.
    This class uses Tkinter to provide a graphical interface where users can specify
    order details and submit them to the order book.

    Attributes:
        master (tk.Tk): The root window for the GUI application.
        orderbook (orderbook.Orderbook): The order book instance created through Pybind11.
        order_id_counter (int): Counter for generating unique order IDs.
        order_type_mapping (dict): Mapping of order type names to integer values used in the order book.
        type_var (tk.StringVar): Variable for holding the order type selected by the user.
        side_var (tk.StringVar): Variable for holding the order side (buy/sell) selected by the user.
        price_entry (tk.Entry): Entry field for inputting the order price.
        qty_entry (tk.Entry): Entry field for inputting the order quantity.
    """

    def __init__(self, master):
        """
        Initializes the OrderEntryGUI with necessary Tkinter widgets and sets up the order book instance.

        Args:
            master (tk.Tk): The main window in which the GUI is displayed.
        """
        self.master = master
        self.orderbook = orderbook.Orderbook()  # Initialize the order book from the orderbook module

        # Counter to assign unique IDs to each order
        self.order_id_counter = 0

        # Mapping of order types to integer codes
        self.order_type_mapping = {
            'GoodTillCancel': 1,
            'FillAndKill': 2,
            'Market': 3,
            'GoodForDay': 4,
            'FillOrKill': 5
        }

        # Tkinter variables for capturing user inputs
        self.type_var = tk.StringVar(master)
        self.side_var = tk.StringVar(master)
        self.price_entry = tk.Entry(master)
        self.qty_entry = tk.Entry(master)

        # Set up the widgets in the GUI
        self.create_widgets()

    def create_widgets(self):
        """
        Creates and arranges the Tkinter widgets in the main window.
        Adds labels, input fields, option menus, and buttons for user interaction.
        """
        # Labels for each field
        tk.Label(self.master, text="Type").grid(row=0, column=0)
        tk.Label(self.master, text="Side").grid(row=1, column=0)
        tk.Label(self.master, text="Price").grid(row=2, column=0)
        tk.Label(self.master, text="Quantity").grid(row=3, column=0)

        # Dropdown menu for order types
        types = ['GoodTillCancel', 'FillAndKill', 'Market', 'GoodForDay', 'FillOrKill']
        tk.OptionMenu(self.master, self.type_var, *types).grid(row=0, column=1)

        # Dropdown menu for buy/sell side
        sides = ['Buy', 'Sell']
        tk.OptionMenu(self.master, self.side_var, *sides).grid(row=1, column=1)

        # Entry fields for price and quantity
        self.price_entry.grid(row=2, column=1)
        self.qty_entry.grid(row=3, column=1)

        # Buttons to add order or reset the form
        tk.Button(self.master, text="Add Order", command=self.add_order).grid(row=4, column=0, columnspan=2)
        tk.Button(self.master, text="Reset", command=self.reset).grid(row=5, column=0, columnspan=2)

    def add_order(self):
        """
        Adds an order to the order book with the details provided by the user.
        Validates the input fields, generates a new order ID, and displays a confirmation message.
        If any input is invalid, an error message is shown.
        """
        order_type_str = self.type_var.get()
        order_type = self.order_type_mapping.get(order_type_str)
        side = self.side_var.get()

        try:
            # Convert price and quantity to integers
            price = int(self.price_entry.get())
            qty = int(self.qty_entry.get())
        except ValueError:
            # Show error if price or quantity is not a number
            messagebox.showerror("Error", "Price and quantity must be numbers.")
            return

        # Generate a unique order ID
        order_id = self.order_id_counter
        self.order_id_counter += 1

        # Add the order to the order book (1 for Buy, 2 for Sell based on the selection)
        self.orderbook.addOrder(order_type, order_id, 1 if side == 'Buy' else 2, price, qty)
        print("Order added:", order_type_str, order_id, side, price, qty)

        # Confirmation message upon successful addition
        messagebox.showinfo("Order Added", "Order added successfully.")

        # Reset the input fields for the next entry
        self.type_var.set('')
        self.side_var.set('')
        self.price_entry.delete(0, tk.END)
        self.qty_entry.delete(0, tk.END)

    def reset(self):
        """
        Resets all input fields to their default state, clearing previous entries.
        """
        self.type_var.set('')
        self.side_var.set('')
        self.price_entry.delete(0, tk.END)
        self.qty_entry.delete(0, tk.END)

if __name__ == "__main__":
    # Initialize and run the Tkinter application
    root = tk.Tk()
    root.title("Order Entry")
    app = OrderEntryGUI(root)
    root.mainloop()
