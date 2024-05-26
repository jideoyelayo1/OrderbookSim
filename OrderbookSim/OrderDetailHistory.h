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
    std::unordered_map<OrderId, OrderDetail> _liveOrders;

    std::pair< std::vector<OrderDetail>, std::vector<OrderDetail>> _getLiveOrders() const;    
    

    void addOrderToBuyHistory(const OrderType type, const OrderId id, const Side side, const Price price, const Quantity qty);

    void addOrderToSellHistory(const OrderType type, const OrderId id, const Side side, const Price price, const Quantity qty);

    void saveBuyHistoryToJson(const std::string& filename);
    void saveSellHistoryToJson(const std::string& filename);
    void savePurchaseHistoryToJson(const std::string& filename);

    void _printAHistory(const std::vector<OrderDetail> &history);

public:

    OrderDetailHistory() = default;

    void addOrderToHistory(const OrderType type, const OrderId id, const Side side, const Price price, const Quantity qty);

    void addOrderToPurchaseHistory(const Price price, const Quantity qty);

    void saveHistoryToJson(const std::string& buyfilename = "BuyHistory.json", const std::string& sellfilename = "SellHistory.json", const std::string& purchasefilename = "PurchaseHistory.json");

    void removeMatchedOrder(const OrderId bidId, const OrderId askID);
    
    void _printSellHistory();

    void _printBuyHistory();

    void _printPurchaseHistory();

    void _printLiveOrders();

    Price getVWAP();
    
    Price getVWAP(int n);


    
};


