#include "OrderDetailHistory.h"

#pragma once



    void OrderDetailHistory::addOrderToBuyHistory(OrderType type, OrderId id, Side side, Price price, Quantity qty) {
        OrderDetail newOrderDetail(type, id, side, price, qty);
        buyHistory.push_back(newOrderDetail);
    }

    void OrderDetailHistory::addOrderToSellHistory(OrderType type, OrderId id, Side side, Price price, Quantity qty) {
        OrderDetail newOrderDetail(type, id, side, price, qty);
        sellHistory.push_back(newOrderDetail);
    }




    void OrderDetailHistory::addOrderToHistory(OrderType type, OrderId id, Side side, Price price, Quantity qty) {
        if (side == Side::Buy) {
            addOrderToBuyHistory(type, id, side, price, qty);
        }
        else if (side == Side::Sell) {
            addOrderToSellHistory(type, id, side, price, qty);
        }
    }
    void OrderDetailHistory::addOrderToPurchaseHistory(Price price, Quantity qty) {
        auto newPurchasedOrder = MatchedOrderDetail(price, qty);
        _purchaseHistory.push_back(newPurchasedOrder);
    }


    void OrderDetailHistory::_printSellHistory() {
        std::cout << "Sell Order History" << std::endl;
        for (const OrderDetail detail : sellHistory) {
            std::cout << "OrderType: " << detail.getOrderType() << " ";
            std::cout << "Id: " << detail.getOrderId() << " ";
            std::cout << "Side: " << detail.getSide() << " ";
            std::cout << "Price: " << detail.getPrice() << " ";
            std::cout << "Quantity: " << detail.getQuantity() << " ";
            std::cout << "Time: " << detail.getTime() << std::endl;
        }
        std::cout << std::endl;

    }

    void OrderDetailHistory::_printBuyHistory() {
        std::cout << "Buy Order History" << std::endl;
        for (const OrderDetail detail : buyHistory) {
            std::cout << "OrderType: " << detail.getOrderType() << " ";
            std::cout << "Id: " << detail.getOrderId() << " ";
            std::cout << "Side: " << detail.getSide() << " ";
            std::cout << "Price: " << detail.getPrice() << " ";
            std::cout << "Quantity: " << detail.getQuantity() << " ";
            std::cout << "Time: " << detail.getTime() << std::endl;
        }
        std::cout << std::endl;
    }

    void OrderDetailHistory::_printPurchaseHistory() {
        std::cout << "Purchase History" << std::endl;
        for (const MatchedOrderDetail detail : _purchaseHistory) {
            std::cout << "Price: " << detail.getPrice() << " ";
            std::cout << "Quantity: " << detail.getQuantity() << " ";
            std::cout << "Time: " << detail.getTime() << std::endl;
        }
        std::cout << std::endl;
    }

    Price OrderDetailHistory::getVWAP() {
        if (_purchaseHistory.empty()) return 0;
        Price price = 0;
        long long num = 0;
        long long dom = 0;
        for (MatchedOrderDetail sold : _purchaseHistory) {
            num += sold.getPrice() * sold.getQuantity();
            dom += sold.getQuantity();
        }
        if (dom != 0) price = num / dom;

        return price;
    }

    Price OrderDetailHistory::getVWAP(int n) {
        const int historySize = _purchaseHistory.size();
        if (historySize < n) return getVWAP();

        Price price = 0;
        long long num = 0;
        long long dom = 0;
        for (int i = historySize - 1; i > historySize - 1 - n; i--) {
            num += _purchaseHistory[i].getPrice() * _purchaseHistory[i].getQuantity();
            dom += _purchaseHistory[i].getQuantity();
        }

        if (dom != 0) price = num / dom;

        return price;
    }

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

/*
// Define how to serialize and deserialize OrderDetailHistory to/from JSON
void to_json(nlohmann::json& j, const OrderDetailHistory& history) {
    j = history.toJSON();
}

void from_json(const nlohmann::json& j, OrderDetailHistory& history) {
    history = OrderDetailHistory::fromJSON(j);
}
*/


