/*!
 * @file OrderDetailHistory.hpp
 * @brief Declares the OrderDetailHistory class for managing order histories and live orders.
 *
 * The OrderDetailHistory class provides methods to track and manage historical buy, sell, 
 * and matched orders, save order data to JSON, and update order details using a neural network.
 */

#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include "json.hpp"
#include "NeuralNetwork.hpp"
#include "OrderDetail.hpp"
#include "MatchedOrderDetails.hpp"

/*!
 * @class OrderDetailHistory
 * @brief Manages the history of orders and tracks live orders in the orderbook.
 *
 * The OrderDetailHistory class stores order details, provides methods to save
 * and retrieve order data, and uses a neural network to predict future prices.
 */
class OrderDetailHistory {
private:
    std::vector<OrderDetail> buyHistory;               ///< History of buy orders.
    std::vector<OrderDetail> sellHistory;              ///< History of sell orders.
    std::vector<MatchedOrderDetail> _purchaseHistory;  ///< History of matched purchase orders.
    std::unordered_map<OrderId, OrderDetail> _liveOrders; ///< Map of currently live orders.

    NeuralNetwork neutralNetwork;                      ///< Neural network for predicting prices.
    Price lastPrediction = 0;                          ///< Last predicted price from the neural network.
    Price PRICESCALE = 500;                            ///< Scale factor for price normalization in predictions.

    /*!
     * @brief Retrieves the current live orders separated into buy and sell orders.
     * @return A pair containing vectors of live sell and buy orders.
     */
    std::pair<std::vector<OrderDetail>, std::vector<OrderDetail>> _getLiveOrders() const;

    /*!
     * @brief Adds an order to the buy history and live orders map.
     */
    void addOrderToBuyHistory(const OrderType type, const OrderId id, const Side side, const Price price, const Quantity qty);

    /*!
     * @brief Adds an order to the sell history and live orders map.
     */
    void addOrderToSellHistory(const OrderType type, const OrderId id, const Side side, const Price price, const Quantity qty);

    /*!
     * @brief Saves a history of OrderDetail objects to a JSON file.
     */
    void saveHistoryToJson(const std::string& filename, const std::vector<OrderDetail> history);

    /*!
     * @brief Saves a history of MatchedOrderDetail objects to a JSON file.
     */
    void saveHistoryToJson(const std::string& filename, const std::vector<MatchedOrderDetail> history);

    /*!
     * @brief Loads JSON data for live orders from specified files.
     */
    void _loadJsonLiveOrders(const std::string& SellFilename, const std::string& buyFilename);

    /*!
     * @brief Prints live orders to the console and saves to specified JSON files.
     */
    void _printLiveOrders(const std::string& SellFilename, const std::string& buyFilename);

    /*!
     * @brief Prints a vector of OrderDetail history to the console.
     */
    void _printAHistory(const std::vector<OrderDetail>& history);

    /*!
     * @brief Converts the current time of day to a fraction of the day for use in neural network inputs.
     * @return A double representing the fraction of the day passed.
     */
    double getCurrentTimeAsFractionOfDay();

    /*!
     * @brief Loads price history data into the neural network for training.
     */
    void loadHistoryToNeuralNetwork(std::vector<Price> inputPrice, std::vector<Price> outputPrice);

public:
    /*!
     * @brief Default constructor that initializes the neural network with a predefined topology.
     */
    OrderDetailHistory() {
        const std::vector<unsigned> topology = { 2, 3, 1 };
        neutralNetwork.loadTogology(topology);
    }

    /*!
     * @brief Returns the size of the buy history.
     * @return The number of buy orders in the history.
     */
    int buyHistorySize() const;

    /*!
     * @brief Returns the size of the sell history.
     * @return The number of sell orders in the history.
     */
    int sellHistorySize() const;

    /*!
     * @brief Returns the size of the purchase history.
     * @return The number of matched purchase orders in the history.
     */
    int purchaseHistorySize() const;

    /*!
     * @brief Updates the neural network with the latest order price and quantity data.
     */
    void updateNeutralNetwork(Price price, Quantity quantity);

    /*!
     * @brief Adds an order to the appropriate history (buy or sell) based on its side.
     */
    void addOrderToHistory(const OrderType type, const OrderId id, const Side side, const Price price, const Quantity qty);

    /*!
     * @brief Adds a new purchase order to the purchase history.
     */
    void addOrderToPurchaseHistory(const Price price, const Quantity qty);

    /*!
     * @brief Saves the buy, sell, and purchase histories to JSON files.
     */
    void saveHistoryToJson(const std::string& buyfilename, const std::string& sellfilename, const std::string& purchasefilename);

    /*!
     * @brief Saves multiple histories including live orders to JSON files.
     */
    void saveHistoryToJson(const std::string& buyfilename = "BuyHistory.json", const std::string& sellfilename = "SellHistory.json", const std::string& purchasefilename = "PurchaseHistory.json", const std::string& liveSellOrdersFilename = "LiveSellHistory.json", const std::string& buySellOrdersFilename = "LiveBuyHistory.json");

    /*!
     * @brief Removes matched orders by IDs from the live orders map.
     */
    void removeMatchedOrder(const OrderId bidId, const OrderId askID);

    /*!
     * @brief Prints the sell history to the console.
     */
    void _printSellHistory();

    /*!
     * @brief Prints the buy history to the console.
     */
    void _printBuyHistory();

    /*!
     * @brief Prints the purchase history to the console.
     */
    void _printPurchaseHistory();

    /*!
     * @brief Prints the live orders to the console.
     */
    void _printLiveOrders();

    /*!
     * @brief Calculates the Volume-Weighted Average Price (VWAP) of all purchases.
     * @return The VWAP of all purchases.
     */
    Price getVWAP();

    /*!
     * @brief Calculates the VWAP of the last `n` purchases.
     * @return The VWAP of the last `n` purchases.
     */
    Price getVWAP(int n);

    /*!
     * @brief Retrieves the last prediction from the neural network.
     * @return The predicted price.
     */
    Price getPrediction();

    /*!
     * @brief Retrieves a map of all currently live orders.
     * @return An unordered_map of live orders.
     */
    std::unordered_map<OrderId, OrderDetail> getLiveOrders();

    /*!
     * @brief Deletes a live order by ID.
     */
    void deleteALiveOrder(OrderId id);
};
