# OrderbookSim

## Overview

OrderbookSim is a C++ application that simulates an order book used in financial markets. It manages and matches buy and sell orders efficiently and calculates the volume-weighted average price (VWAP). This project is designed to help understand how order books work and to simulate trading scenarios with different types of orders.

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

## Usage

### Example
Here's a simple example of how to use the order book:

```cpp
#include "Orderbook.h"

int main() {
    Orderbook orderbook;

    // Add a buy order
    const OrderId orderId = 1;
    orderbook.addOrder(std::make_shared<Order>(OrderType::GoodTillCancel, orderId, Side::Buy, 100, 10));

    // Check the size of the order book
    std::cout << "Orderbook size: " << orderbook.Size() << std::endl;

    // Cancel the order
    orderbook.CancelOrder(orderId);

    // Check the size again
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

// Prototyping
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

## Dependencies
The project relies on standard C++ libraries and includes the Google Test framework for testing purposes.

## Testing
The project includes comprehensive unit tests implemented using the Google Test framework to ensure correctness and reliability. To run the tests, use the following file:

- `test.cpp`

## Contributing
Contributions are welcome! Feel free to open issues or pull requests for any improvements or bug fixes.

## License
This project is licensed under the MIT License - see the LICENSE file for details.

## Additional Features
The project now includes an order history and the calculation of the volume-weighted average price (VWAP).

Feel free to customize this README further to provide more detailed information about your project's features, usage, or any other relevant details.