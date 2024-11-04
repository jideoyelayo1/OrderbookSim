import json
import tkinter as tk
from tkinter import ttk

def load_json(filename):
    """
    Loads data from a JSON file.

    Args:
        filename (str): The path to the JSON file.

    Returns:
        list: A list of dictionaries, each representing a record from the JSON file.
    """
    with open(filename, 'r') as file:
        return json.load(file)

def create_table(frame, headers, data):
    """
    Creates a table in a Tkinter frame using a Treeview widget to display data.

    Args:
        frame (tk.Frame): The frame in which the table will be created.
        headers (list of str): Column headers for the table.
        data (list of dict): Data to populate the table, with each dictionary representing a row.

    Returns:
        ttk.Treeview: The created Treeview table widget.
    """
    # Create a Treeview widget with specified headers
    tree = ttk.Treeview(frame, columns=headers, show='headings')
    
    # Configure each column with a header and width
    for header in headers:
        tree.heading(header, text=header)
        tree.column(header, width=100)

    # Insert data into the table row by row
    for row in data:
        tree.insert('', 'end', values=[row.get(header, '') for header in headers])

    # Pack the Treeview widget to fill available space in the frame
    tree.pack(expand=True, fill='both')
    return tree

def refresh_data():
    """
    Refreshes data by loading updated JSON files and updating each table.
    This function also schedules itself to be called every 10 seconds.
    """
    global sell_orders, buy_orders, purchases, live_sell_orders, live_buy_orders

    # Load data from JSON files for each table
    sell_orders = load_json('SellHistory.json')
    buy_orders = load_json('BuyHistory.json')
    purchases = load_json('PurchaseHistory.json')
    live_sell_orders = load_json('SellLiveHistory.json')
    live_buy_orders = load_json('BuyLiveHistory.json')

    # Update each table with the loaded data
    update_table(frame_sell_orders, sell_order_headers, sell_orders)
    update_table(frame_buy_orders, buy_order_headers, buy_orders)
    update_table(frame_purchases, purchase_headers, purchases)
    update_table(frame_live_sell_orders, live_sell_order_headers, live_sell_orders)
    update_table(frame_live_buy_orders, live_buy_order_headers, live_buy_orders)

    # Schedule the next refresh after 10 seconds
    root.after(10000, refresh_data)

def update_table(frame, headers, data):
    """
    Clears and recreates a table within a specified frame to display updated data.

    Args:
        frame (tk.Frame): The frame containing the table to update.
        headers (list of str): Column headers for the table.
        data (list of dict): New data to display in the table.
    """
    # Destroy all existing widgets in the frame
    for widget in frame.winfo_children():
        widget.destroy()
    
    # Re-create the table with updated data
    create_table(frame, headers, data)

# Initialize the main application window
root = tk.Tk()
root.title("Order Book")
root.geometry("1000x600")

# Create a notebook to organize multiple tables into tabs
notebook = ttk.Notebook(root)
notebook.pack(expand=True, fill="both")

# Create frames for each category of orders
frame_sell_orders = tk.Frame(notebook)
frame_buy_orders = tk.Frame(notebook)
frame_purchases = tk.Frame(notebook)
frame_live_sell_orders = tk.Frame(notebook)
frame_live_buy_orders = tk.Frame(notebook)

# Add frames to the notebook with appropriate labels
notebook.add(frame_sell_orders, text="Sell Orders")
notebook.add(frame_buy_orders, text="Buy Orders")
notebook.add(frame_purchases, text="Purchases")
notebook.add(frame_live_sell_orders, text="Live Sell Orders")
notebook.add(frame_live_buy_orders, text="Live Buy Orders")

# Define headers for each table based on data structure
sell_order_headers = ["order_id", "order_type", "price", "quantity", "side", "time"]
buy_order_headers = ["order_id", "order_type", "price", "quantity", "side", "time"]
purchase_headers = ["price", "quantity", "time"]
live_sell_order_headers = ["order_id", "order_type", "price", "quantity", "side", "time"]
live_buy_order_headers = ["order_id", "order_type", "price", "quantity", "side", "time"]

# Initialize data in each table and start the data refresh cycle
refresh_data()

# Run the Tkinter main loop to display the GUI
root.mainloop()
