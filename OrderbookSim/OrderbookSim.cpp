/*!
 * @file OrderbookSim.cpp
 * @brief Contains the main function for simulating an orderbook with random orders.
 *
 * This file initializes an Orderbook instance and performs a simulation by adding
 * randomly generated orders. It demonstrates the functionality of the Orderbook
 * class, including adding orders, printing the orderbook, and saving it to JSON.
 */

#include <iostream>
#include "Orderbook.hpp"
#include <random>

// Function prototype for the order simulation function
/*!
 * @brief Simulates random order entries in the orderbook.
 *
 * @param n The number of random orders to simulate.
 */
void randomOrderSimulation(const int n);

/*!
 * @brief The main function where program execution begins.
 *
 * Initializes the orderbook simulation with a predefined number of random orders.
 *
 * @return An integer representing the program's exit status.
 */
int main()
{
    // Simulation parameters
    const int n = 100;
    randomOrderSimulation(n);

    return 0;
}

/*!
 * @brief Simulates the addition of random orders to an Orderbook instance.
 *
 * Generates `n` random orders with randomized types, sides, prices, and quantities.
 * After adding all orders, it prints the orderbook and saves its data to JSON.
 *
 * @param n The number of random orders to add to the orderbook.
 */
void randomOrderSimulation(const int n) {
    Orderbook orderbook; ///< Orderbook instance for the simulation.

    // Random number generation setup
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> type(1, 5);      ///< Distribution for random order types.
    std::uniform_int_distribution<> side(1, 100);    ///< Distribution for random order sides.
    std::uniform_int_distribution<> price(1, 500);   ///< Distribution for random order prices.
    std::uniform_int_distribution<> quantity(1, 500);///< Distribution for random order quantities.

    // Loop to generate and add random orders
    for (int i = 0; i < n; i++) {
        orderbook.addOrder(intToOrdertType(type(gen)), i, side(gen) == 1 ? Side::Buy : Side::Sell, price(gen), quantity(gen));

        // Optional delay for monitoring large simulations (commented out in default usage)
        if (false && i % 1000 == 0) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << ".";
        }
    }
    std::cout << std::endl;

    // Print orderbook details and save data to JSON
    orderbook.printAllOrders();
    std::cout << "OrderBook Size: " << orderbook.Size() << std::endl;
    orderbook.saveToJson();
}
