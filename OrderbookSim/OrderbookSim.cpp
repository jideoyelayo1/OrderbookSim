// OrderbookSim.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "Orderbook.hpp"

#include <random>

//prototyping
void randomOrderSimulation(const int n);

int main()
{

    /*
    Orderbook orderbook;
    orderbook.addOrder(OrderType::GoodTillCancel, Side::Buy, 53, 32);
    orderbook.addOrder(OrderType::GoodTillCancel, Side::Sell, 42, 43);
    orderbook.addOrder(OrderType::GoodTillCancel, Side::Sell, 31, 21);
    orderbook.addOrder(OrderType::GoodTillCancel, Side::Sell, 53, 95);
    orderbook.addOrder(OrderType::GoodTillCancel, Side::Sell, 32, 35);
    orderbook.addOrder(OrderType::GoodTillCancel, Side::Buy, 324, 65);
    orderbook.printAllOrders();
    return 0;
    */
    const int n = 100;
    randomOrderSimulation(n);
    

}


void randomOrderSimulation(const int n) {
    Orderbook orderbook;
    /*
    Processed 100000 randoms in 5.02383
    Processed 10000 randoms in 0.0998076

    */
    

    std::random_device rd;

    std::mt19937 gen(rd());

    std::uniform_int_distribution<> type(1, 5);
    std::uniform_int_distribution<> side(1, 100);
    std::uniform_int_distribution<> price(1, 500);
    std::uniform_int_distribution<> quantity(1, 500);

    //auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < n; i++) {
        orderbook.addOrder(intToOrdertType(type(gen)), i, side(gen) == 1 ? Side::Buy : Side::Sell, price(gen), quantity(gen));
        if(false and i % 1000 == 0) 
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << ".";
        }
    }
        std::cout << std::endl;
    //auto end = std::chrono::high_resolution_clock::now();

    //std::chrono::duration<double> duration = end - start;
    //std::cout << "Elapsed time: " << duration.count() << " seconds" << std::endl;

    //return;

    //orderbook.EndOfDay(); // Run at the end of Day

    orderbook.printAllOrders();

    std::cout << "OrderBook Size " << orderbook.Size();

    orderbook.saveToJson();
}