#pragma once

#include <vector>
#include <iostream>
#include  "json.hpp"
#include <fstream> 

#include "NeuralNetwork.h"

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

    NeuralNetwork neutralNetwork;
    

    void addOrderToBuyHistory(const OrderType type, const OrderId id, const Side side, const Price price, const Quantity qty);

    void addOrderToSellHistory(const OrderType type, const OrderId id, const Side side, const Price price, const Quantity qty);

    void saveBuyHistoryToJson(const std::string& filename);
    void saveSellHistoryToJson(const std::string& filename);
    void savePurchaseHistoryToJson(const std::string& filename);

    Price lastPrediction = 0;

    Price PRICESCALE = 1;

    

    void _printAHistory(const std::vector<OrderDetail> &history);

    double getCurrentTimeAsFractionOfDay() {
        // Get the current time point
        auto now = std::chrono::system_clock::now();

        // Convert to time_t to extract the time of day
        time_t now_c = std::chrono::system_clock::to_time_t(now);

        // Define a tm structure to hold the local time
        tm local_tm;

        // Use localtime_s to convert time_t to tm structure
        localtime_s(&local_tm, &now_c);

        // Calculate the number of seconds since midnight
        int seconds_since_midnight = local_tm.tm_hour * 3600 + local_tm.tm_min * 60 + local_tm.tm_sec;

        // Total number of seconds in a day
        const int seconds_per_day = 24 * 3600;

        // Scale to a range of 0 to 1
        double fraction_of_day = static_cast<double>(seconds_since_midnight) / seconds_per_day;

        return fraction_of_day;
    }

    void loadHistoryToNeuralNetwork(std::vector<Price> inputPrice, std::vector<Price> outputPrice);

    void updateNeutralNetwork( Price price, Quantity quantity) {
        loadHistoryToNeuralNetwork({ static_cast<int> (getCurrentTimeAsFractionOfDay()) * 100,static_cast<int>(quantity) }, { price });
        std::cout << "The predicted price is " << getPrediction() << std::endl;
    }

public:

    //OrderDetailHistory() = default;

    OrderDetailHistory() {
        const std::vector<unsigned> topology = { 2,3,1 };
        neutralNetwork.loadTogology(topology);
    }

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


    Price getPrediction() { return lastPrediction; }


    
};


