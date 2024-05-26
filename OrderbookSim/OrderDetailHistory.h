#pragma once

#include <vector>
#include <iostream>
#include  "json.hpp"
#include <fstream> 

//#include <nlohmann/json.hpp>
#include "OrderDetail.h"
#include "MatchedOrderDetails.h"

class OrderDetailHistory {
private:
    std::vector<OrderDetail> buyHistory;
    std::vector<OrderDetail> sellHistory;
    std::vector <MatchedOrderDetail> _purchaseHistory;

    
    

    void addOrderToBuyHistory(OrderType type, OrderId id, Side side, Price price, Quantity qty);

    void addOrderToSellHistory(OrderType type, OrderId id, Side side, Price price, Quantity qty);

public:
    OrderDetailHistory() = default;

    void addOrderToHistory(OrderType type, OrderId id, Side side, Price price, Quantity qty);

    void addOrderToPurchaseHistory(Price price, Quantity qty);


    void _printSellHistory();

    void _printBuyHistory();

    void _printPurchaseHistory();

    Price getVWAP();
    
    Price getVWAP(int n);

void saveBuyHistoryToJson(const std::string& filename);
void saveSellHistoryToJson(const std::string& filename);
void savePurchaseHistoryToJson(const std::string& filename);

    
};


