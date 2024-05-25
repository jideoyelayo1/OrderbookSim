// OrderbookSim.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "Orderbook.h"

int main()
{
    Orderbook orderbook;

    const OrderId orderId = 1;
    /*
    orderbook.addOrder(std::make_shared<Order>(OrderType::GoodTillCancel, 1, Side::Buy, 70, 10));
    orderbook.addOrder(std::make_shared<Order>(OrderType::GoodTillCancel, 2, Side::Sell, 100, 10));
    orderbook.addOrder(std::make_shared<Order>(OrderType::GoodTillCancel, 2, Side::Buy, 70, 21));
    orderbook.addOrder(std::make_shared<Order>(OrderType::GoodTillCancel, 2, Side::Sell, 140, 95));
    */
    orderbook.addOrder(OrderType::GoodTillCancel, Side::Sell, 50, 10);
    orderbook.addOrder(OrderType::GoodTillCancel, Side::Buy, 100, 10);
    
    orderbook.printAllOrders();

    std::cout << "OrderBook Size " << orderbook.Size();

    //std::cout << "Hello World!\n";
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
