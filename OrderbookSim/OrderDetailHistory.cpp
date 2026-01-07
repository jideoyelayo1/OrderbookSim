/*!
 * @file OrderDetailHistory.cpp
 * @brief Implements the OrderDetailHistory class, providing functionality to manage and track order history.
 *
 * This file contains the implementation of the OrderDetailHistory class, which stores and manages
 * historical and live orders, as well as offers methods to handle matched orders, save histories to JSON,
 * and update predictions with a neural network.
 */

#include "OrderDetailHistory.hpp"

/*!
 * @brief Adds an order to the buy history and the live order map.
 *
 * @param type The type of the order.
 * @param id The unique identifier of the order.
 * @param side The side of the order (Buy or Sell).
 * @param price The price of the order.
 * @param qty The quantity of the order.
 */
void OrderDetailHistory::addOrderToBuyHistory(const OrderType type, const OrderId id, const Side side, const Price price, const Quantity qty) {
    OrderDetail newOrderDetail(type, id, side, price, qty);
    _liveOrders[id] = newOrderDetail;
    buyHistory.push_back(newOrderDetail);
}

/*!
 * @brief Adds an order to the sell history and the live order map.
 *
 * @param type The type of the order.
 * @param id The unique identifier of the order.
 * @param side The side of the order (Buy or Sell).
 * @param price The price of the order.
 * @param qty The quantity of the order.
 */
void OrderDetailHistory::addOrderToSellHistory(const OrderType type, const OrderId id, const Side side, const Price price, const Quantity qty) {
    OrderDetail newOrderDetail(type, id, side, price, qty);
    _liveOrders[id] = newOrderDetail;
    sellHistory.push_back(newOrderDetail);
}

/*!
 * @brief Removes a matched order from the live order map.
 *
 * @param bidId The unique identifier of the buy order.
 * @param askId The unique identifier of the sell order.
 */
void OrderDetailHistory::removeMatchedOrder(const OrderId bidId, const OrderId askId) {
    if (_liveOrders.find(bidId) == _liveOrders.end() || _liveOrders.find(askId) == _liveOrders.end()) return;

    Quantity bid = _liveOrders[bidId].getQuantity();
    Quantity ask = _liveOrders[askId].getQuantity();

    if (bid == ask) {
        _liveOrders.erase(askId);
        _liveOrders.erase(bidId);
    } else if (bid > ask) {
        bid -= ask;
        _liveOrders.erase(askId);
        _liveOrders[bidId].setQuantity(bid);
    } else if (ask > bid) {
        ask -= bid;
        _liveOrders.erase(bidId);
        _liveOrders[askId].setQuantity(ask);
    }
}

/*!
 * @brief Adds an order to the appropriate history (buy or sell) based on its side.
 *
 * @param type The type of the order.
 * @param id The unique identifier of the order.
 * @param side The side of the order (Buy or Sell).
 * @param price The price of the order.
 * @param qty The quantity of the order.
 */
void OrderDetailHistory::addOrderToHistory(const OrderType type, const OrderId id, const Side side, const Price price, const Quantity qty) {
    if (side == Side::Buy) {
        addOrderToBuyHistory(type, id, side, price, qty);
    } else if (side == Side::Sell) {
        addOrderToSellHistory(type, id, side, price, qty);
    }
}

/*!
 * @brief Adds a new purchase to the purchase history.
 *
 * @param price The price at which the order was purchased.
 * @param qty The quantity of the order.
 */
void OrderDetailHistory::addOrderToPurchaseHistory(const Price price, const Quantity qty) {
    auto newPurchasedOrder = MatchedOrderDetail(price, qty);
    _purchaseHistory.push_back(newPurchasedOrder);
}

/*!
 * @brief Prints the sell history.
 */
void OrderDetailHistory::_printSellHistory() {
    std::cout << "Sell Order History" << std::endl;
    _printAHistory(sellHistory);
}

/*!
 * @brief Prints the buy history.
 */
void OrderDetailHistory::_printBuyHistory() {
    std::cout << "Buy Order History" << std::endl;
    _printAHistory(buyHistory);
}

/*!
 * @brief Prints the purchase history.
 */
void OrderDetailHistory::_printPurchaseHistory() {
    std::cout << "Purchase History" << std::endl;
    for (const MatchedOrderDetail& detail : _purchaseHistory) {
        std::cout << "Price: " << detail.getPrice() << " "
                  << "Quantity: " << detail.getQuantity() << " "
                  << "Time: " << detail.getTime() << std::endl;
    }
    std::cout << std::endl;
}

/*!
 * @brief Calculates the Volume-Weighted Average Price (VWAP) of all purchases.
 *
 * @return The VWAP of all purchases.
 */
Price OrderDetailHistory::getVWAP() {
    if (_purchaseHistory.empty()) return 0;
    long long num = 0, dom = 0;
    for (const MatchedOrderDetail& sold : _purchaseHistory) {
        num += sold.getPrice() * sold.getQuantity();
        dom += sold.getQuantity();
    }
    return dom != 0 ? num / dom : 0;
}

/*!
 * @brief Calculates the VWAP for the last `n` purchases.
 *
 * @param n The number of recent purchases to include in the calculation.
 * @return The VWAP of the last `n` purchases.
 */
Price OrderDetailHistory::getVWAP(int n) {
    const int historySize = _purchaseHistory.size();
    if (historySize < n) return getVWAP();

    long long num = 0, dom = 0;
    for (int i = historySize - 1; i >= historySize - n; --i) {
        num += _purchaseHistory[i].getPrice() * _purchaseHistory[i].getQuantity();
        dom += _purchaseHistory[i].getQuantity();
    }
    return dom != 0 ? num / dom : 0;
}

/*!
 * @brief Saves a history of orders to a JSON file.
 *
 * @param filename The name of the JSON file to save.
 * @param history The order history to save.
 */
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

/*!
 * @brief Retrieves the current VWAP price.
 *
 * @return The most recent prediction from the neural network.
 */
Price OrderDetailHistory::getPrediction() {
    return lastPrediction;
}

/*!
 * @brief Returns a copy of the live orders map.
 *
 * @return A map of live orders indexed by order ID.
 */
std::unordered_map<OrderId, OrderDetail> OrderDetailHistory::getLiveOrders() {
    return _liveOrders;
}

/*!
 * @brief Deletes a live order by its ID.
 *
 * @param id The ID of the order to delete.
 */
void OrderDetailHistory::deleteALiveOrder(OrderId id) {
    if (_liveOrders.find(id) != _liveOrders.end()) _liveOrders.erase(id);
}

/*!
 * @brief Updates the neural network with new price and quantity data.
 *
 * @param price The price of the latest matched order.
 * @param quantity The quantity of the latest matched order.
 */
void OrderDetailHistory::updateNeutralNetwork(Price price, Quantity quantity) {
    loadHistoryToNeuralNetwork({ static_cast<int>(getCurrentTimeAsFractionOfDay()) * 100, static_cast<int>(quantity) }, { price });
}

/*!
 * @brief prints an order detail.
 *
 * @param history A vector of order details.
 */
void OrderDetailHistory::_printAHistory(const std::vector<OrderDetail>& history) {
    for (const OrderDetail& orderDetail : history) {
        std::cout << "Price: " << orderDetail.getPrice() << " "
                  << "Quantity: " << orderDetail.getQuantity() << " "
                  << "Time: " << orderDetail.getTime() << std::endl;
    }
    std::cout << std::endl;
}


// 5-file save wrapper used by Orderbook::saveToJson
void OrderDetailHistory::saveHistoryToJson(
    const std::string& buyfilename,
    const std::string& sellfilename,
    const std::string& purchasefilename,
    const std::string& sellLivefilename,
    const std::string& buyLivefilename)
{
    // Buy and sell history
    saveHistoryToJson(buyfilename, buyHistory);
    saveHistoryToJson(sellfilename, sellHistory);

    // Purchase history: MatchedOrderDetail vector
    nlohmann::json purchaseJson = nlohmann::json::array();
    for (const MatchedOrderDetail& detail : _purchaseHistory) {
        purchaseJson.push_back({
            {"price", detail.getPrice()},
            {"quantity", detail.getQuantity()},
            {"time", detail.getTime()}
        });
    }
    {
        std::ofstream f(purchasefilename);
        f << purchaseJson.dump(4);
    }

    // Live orders split into buy / sell JSON
    nlohmann::json liveSell = nlohmann::json::array();
    nlohmann::json liveBuy  = nlohmann::json::array();

    for (const auto& [id, detail] : _liveOrders) {
        nlohmann::json obj = {
            {"order_type", detail.getOrderType()},
            {"order_id",   detail.getOrderId()},
            {"side",       detail.getSide()},
            {"price",      detail.getPrice()},
            {"quantity",   detail.getQuantity()},
            {"time",       detail.getTime()}
        };

        if (detail.Side() == Side::Sell)
            liveSell.push_back(obj);
        else
            liveBuy.push_back(obj);
    }

    {
        std::ofstream f(sellLivefilename);
        f << liveSell.dump(4);
    }
    {
        std::ofstream f(buyLivefilename);
        f << liveBuy.dump(4);
    }
}

// Simple sizes for histories
std::size_t OrderDetailHistory::buyHistorySize() const {
    return buyHistory.size();
}

std::size_t OrderDetailHistory::sellHistorySize() const {
    return sellHistory.size();
}

std::size_t OrderDetailHistory::purchaseHistorySize() const {
    return _purchaseHistory.size();
}

// Time of day in [0,1)
double OrderDetailHistory::getCurrentTimeAsFractionOfDay() {
    using namespace std::chrono;

    auto now   = system_clock::now();
    auto now_c = system_clock::to_time_t(now);
    std::tm tm{};

#if defined(_WIN32)
    localtime_s(&tm, &now_c);
#else
    localtime_r(&now_c, &tm);
#endif

    int secondsSinceMidnight =
        tm.tm_hour * 3600 +
        tm.tm_min  * 60 +
        tm.tm_sec;

    return static_cast<double>(secondsSinceMidnight) / 86400.0;
}

// Minimal NN hook so things link and lastPrediction is updated
void OrderDetailHistory::loadHistoryToNeuralNetwork(
    std::vector<int> inputs,
    std::vector<int> outputs)
{
    // TODO: wire into real Net / NeuralNetwork if you want NN active.
    // For now just keep lastPrediction reasonable.
    (void)inputs;
    if (!outputs.empty()) {
        lastPrediction = outputs.back();
    }
}
