#pragma once

#include <vector>
#include <iostream>

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

    // Method to convert OrderDetailHistory to JSON
    /*
    nlohmann::json toJSON() const {
        return nlohmann::json{ {"buyHistory", buyHistory}, {"sellHistory", sellHistory} };
    }
    

    // Method to populate OrderDetailHistory from JSON
    static OrderDetailHistory fromJSON(const nlohmann::json& j) {
        OrderDetailHistory history;
        j.at("buyHistory").get_to(history.buyHistory);
        j.at("sellHistory").get_to(history.sellHistory);
        return history;
    }
    */
};
/*
// Define how to serialize and deserialize OrderDetailHistory to/from JSON
void to_json(nlohmann::json& j, const OrderDetailHistory& history) {
    j = history.toJSON();
}

void from_json(const nlohmann::json& j, OrderDetailHistory& history) {
    history = OrderDetailHistory::fromJSON(j);
}
*/


