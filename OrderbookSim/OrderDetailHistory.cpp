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

    // Save histories to JSON
    void OrderDetailHistory::saveBuyHistoryToJson(const std::string& filename) {
        nlohmann::json jsonBuyHistory = nlohmann::json::array();
        for (const OrderDetail& detail : buyHistory) {
            jsonBuyHistory.push_back({
                {"order_type", detail.getOrderType()},
                {"order_id", detail.getOrderId()},
                {"side", detail.getSide()},
                {"price", detail.getPrice()},
                {"quantity", detail.getQuantity()},
                {"time", detail.getTime()}
                });
        }
        std::ofstream file(filename);
        file << jsonBuyHistory.dump(4);
    }

    void OrderDetailHistory::saveSellHistoryToJson(const std::string& filename) {
        nlohmann::json jsonSellHistory = nlohmann::json::array();
        for (const OrderDetail& detail : sellHistory) {
            jsonSellHistory.push_back({
                {"order_type", detail.getOrderType()},
                {"order_id", detail.getOrderId()},
                {"side", detail.getSide()},
                {"price", detail.getPrice()},
                {"quantity", detail.getQuantity()},
                {"time", detail.getTime()}
                });
        }
        std::ofstream file(filename);
        file << jsonSellHistory.dump(4);
    }

    void OrderDetailHistory::savePurchaseHistoryToJson(const std::string& filename) {
        nlohmann::json jsonPurchaseHistory = nlohmann::json::array();
        for (const MatchedOrderDetail& detail : _purchaseHistory) {
            jsonPurchaseHistory.push_back({
                {"price", detail.getPrice()},
                {"quantity", detail.getQuantity()},
                {"time", detail.getTime()}
                });
        }
        std::ofstream file(filename);
        file << jsonPurchaseHistory.dump(4);
    }

    void OrderDetailHistory::saveHistoryToJson(const std::string& buyfilename, const std::string& sellfilename, const std::string& purchasefilename) {
        saveBuyHistoryToJson(buyfilename);
        saveSellHistoryToJson(sellfilename);
        savePurchaseHistoryToJson(purchasefilename);
    }