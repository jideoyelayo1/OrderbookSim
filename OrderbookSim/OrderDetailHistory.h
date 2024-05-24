#pragma once

#include <vector>
#include <nlohmann/json.hpp>
#include "OrderDetail.h"

class OrderDetailHistory {
private:
    std::vector<OrderDetail> buyHistory;
    std::vector<OrderDetail> sellHistory;

    void addOrderToBuyHistory(OrderType type, OrderId id, Side side, Price price, Quantity qty) {
        OrderDetail newOrderDetail(type, id, side, price, qty);
        buyHistory.push_back(newOrderDetail);
    }

    void addOrderToSellHistory(OrderType type, OrderId id, Side side, Price price, Quantity qty) {
        OrderDetail newOrderDetail(type, id, side, price, qty);
        sellHistory.push_back(newOrderDetail);
    }

public:
    OrderDetailHistory() = default;

    void addOrderToHistory(OrderType type, OrderId id, Side side, Price price, Quantity qty) {
        if (side == Side::Buy) {
            addOrderToBuyHistory(type, id, side, price, qty);
        }
        else if (side == Side::Sell) {
            addOrderToSellHistory(type, id, side, price, qty);
        }
    }

    // Method to convert OrderDetailHistory to JSON
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
};

// Define how to serialize and deserialize OrderDetailHistory to/from JSON
void to_json(nlohmann::json& j, const OrderDetailHistory& history) {
    j = history.toJSON();
}

void from_json(const nlohmann::json& j, OrderDetailHistory& history) {
    history = OrderDetailHistory::fromJSON(j);
}
