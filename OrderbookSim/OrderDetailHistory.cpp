#include "OrderDetailHistory.h"

#pragma once



    void OrderDetailHistory::addOrderToBuyHistory(const OrderType type, const OrderId id, const Side side, const Price price, const Quantity qty) {
        OrderDetail newOrderDetail(type, id, side, price, qty);
        _liveOrders[id] = newOrderDetail;
        buyHistory.push_back(newOrderDetail);
    }

    void OrderDetailHistory::addOrderToSellHistory(const OrderType type, const OrderId id, const Side side, const Price price, const Quantity qty) {
        OrderDetail newOrderDetail(type, id, side, price, qty);
        _liveOrders[id] = newOrderDetail;
        sellHistory.push_back(newOrderDetail);
    }


    void OrderDetailHistory::removeMatchedOrder(const OrderId bidId, const OrderId askId) {

        if (_liveOrders.find(bidId) == _liveOrders.end()) return;
        if (_liveOrders.find(askId) == _liveOrders.end()) return;


        Quantity bid = _liveOrders[bidId].getQuantity();
        Quantity ask = _liveOrders[askId].getQuantity();


        
        if (bid == ask) {
            _liveOrders.erase(askId);
            _liveOrders.erase(bidId);
        }else if (bid > ask) {
            bid -= ask;
            _liveOrders.erase(askId);
            _liveOrders[bidId].setQuantity(bid);
        }else if (ask > bid) {
            ask -= bid;
            _liveOrders.erase(bidId);
            _liveOrders[askId].setQuantity(ask);
        }
        
    }


    void OrderDetailHistory::addOrderToHistory(const OrderType type, const OrderId id, const Side side, const Price price, const Quantity qty) {
        if (side == Side::Buy) {
            addOrderToBuyHistory(type, id, side, price, qty);
        }
        else if (side == Side::Sell) {
            addOrderToSellHistory(type, id, side, price, qty);
        }
    }
    void OrderDetailHistory::addOrderToPurchaseHistory(const Price price, const Quantity qty) {
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

    void OrderDetailHistory::_printAHistory(const std::vector<OrderDetail> &history) {

        for (const OrderDetail detail : history) {
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
    void OrderDetailHistory::saveHistoryToJson(const std::string& filename, const std::vector<OrderDetail> history) {
        nlohmann::json jsonHistory = nlohmann::json::array();

        for (const OrderDetail& detail : history) {
            jsonHistory.push_back({
                {"order_type", detail.getOrderType()},
                {"order_id", detail.getOrderId()},
                {"side", detail.getSide()},
                {"price", detail.getPrice()},
                {"quantity", detail.getQuantity()},
                {"time", detail.getTime()}
                });
        }
        std::ofstream file(filename);
        file << jsonHistory.dump(4);
    }


    void OrderDetailHistory::saveHistoryToJson(const std::string& filename, const std::vector<MatchedOrderDetail> history) {
        nlohmann::json jsonHistory = nlohmann::json::array();
        for (const MatchedOrderDetail& detail : history) {
            jsonHistory.push_back({
                {"price", detail.getPrice()},
                {"quantity", detail.getQuantity()},
                {"time", detail.getTime()}
                });
        }
        std::ofstream file(filename);
        file << jsonHistory.dump(4);
    }


    void OrderDetailHistory::saveHistoryToJson(const std::string& buyfilename, const std::string& sellfilename, const std::string& purchasefilename) {
        saveHistoryToJson(buyfilename, buyHistory);
        saveHistoryToJson(sellfilename, sellHistory);
        saveHistoryToJson(purchasefilename, _purchaseHistory);

    }

    void OrderDetailHistory::saveHistoryToJson(const std::string& buyfilename, const std::string& sellfilename, const std::string& purchasefilename, const std::string& liveSellOrdersFilename, const std::string& buySellOrdersFilename) {
        saveHistoryToJson(buyfilename, buyHistory);
        saveHistoryToJson(sellfilename, sellHistory);
        saveHistoryToJson(purchasefilename, _purchaseHistory);
        _printLiveOrders(liveSellOrdersFilename, buySellOrdersFilename);

    }


    std::pair< std::vector<OrderDetail>, std::vector<OrderDetail>> OrderDetailHistory::_getLiveOrders() const {
        std::vector<OrderDetail> _currentLiveBuys, _currentLiveSells;
        for (const std::pair<OrderId, OrderDetail> &p : _liveOrders) {
            if (p.second.Side() == Side::Buy) { _currentLiveBuys.push_back(p.second); }
            else if (p.second.Side() == Side::Sell) { _currentLiveSells.push_back(p.second); }
        }
        return std::make_pair(_currentLiveSells, _currentLiveBuys);
    }

    void OrderDetailHistory::_loadJsonLiveOrders(const std::string& SellFilename, const std::string& buyFilename) {
        const auto [liveSells, liveBuys] = _getLiveOrders();
        saveHistoryToJson(SellFilename, liveSells);
        saveHistoryToJson(buyFilename, liveBuys);

    }

    void OrderDetailHistory::_printLiveOrders() {
        const auto [liveSells, liveBuys] = _getLiveOrders();

        std::cout << "Live Sell Orders" << std::endl;
        _printAHistory(liveSells);
        std::cout << "Live Buys Orders" << std::endl;
        _printAHistory(liveBuys);

    }

    void OrderDetailHistory::_printLiveOrders(const std::string& SellFilename, const std::string& buyFilename) {
        const auto [liveSells, liveBuys] = _getLiveOrders();
        saveHistoryToJson(SellFilename, liveSells);
        saveHistoryToJson(buyFilename, liveBuys);

        std::cout << "Live Sell Orders" << std::endl;
        _printAHistory(liveSells);
        std::cout << "Live Buys Orders" << std::endl;
        _printAHistory(liveBuys);

    }


    void OrderDetailHistory::loadHistoryToNeuralNetwork(std::vector<Price> inputPrice, std::vector<Price> outputPrice) {

        /*SHOULD ADD DIFFERENT TYPES OF SCALES*/
        Price SCALE = PRICESCALE;
        //Price SCALE = PRICESCALE;


        std::vector<double> inputs(inputPrice.size());
        std::vector<double> targets(outputPrice.size());

        std::transform(inputPrice.begin(), inputPrice.end(), inputs.begin(), [SCALE](Price p) { return static_cast<double>(p) / SCALE; });
        std::transform(outputPrice.begin(), outputPrice.end(), targets.begin(), [SCALE](Price p) { return static_cast<double>(p) / SCALE; });

        std::vector<double> results;

        neutralNetwork.Run(inputs, targets, results);

        if (!results.empty()) {
            lastPrediction = results[0]*SCALE;
            //std::cout << lastPrediction << "\n";
        }
    }


