/*!
 * @file Orderbook.hpp
 * @brief Defines the Orderbook class for managing orders and trades in a trading system.
 *
 * This file contains the Orderbook class, which handles the management, matching,
 * and cancellation of orders. It includes private and public member functions for
 * maintaining an orderbook, updating levels, handling Good For Day orders, and
 * saving order data to JSON files.
 */

#pragma once

#include "Order.hpp"
#include "Trade.hpp"
#include "Usings.hpp"
#include "OrderModify.hpp"
#include "OrderbookLevelInfos.hpp"
#include "OrderDetailHistory.hpp"

#include <map>
#include <unordered_map>
#include <optional>
#include <mutex>
#include <condition_variable>
#include <cmath>

/*!
 * @class Orderbook
 * @brief Manages and matches orders in a trading environment.
 *
 * The Orderbook class maintains separate collections for bids and asks, as well
 * as provides functions for adding, canceling, modifying, and matching orders.
 * It also includes features for managing Good For Day orders and saving the
 * orderbook state to JSON.
 */
class Orderbook {
private:
    /*!
     * @struct OrderEntry
     * @brief Holds a pointer to an order and its location within a collection.
     */
    struct OrderEntry {
        OrderPtr _order{ nullptr }; ///< Pointer to the order.
        OrderPtrs::iterator _location; ///< Iterator to the order's location.
    };

    /*!
     * @enum ExecutionTypes
     * @brief Specifies the type of price execution used in matching trades.
     */
    enum class ExecutionTypes {
        BuyersPrice,   ///< Execution at the buyer's price.
        SellersPrice,  ///< Execution at the seller's price.
        MidPrice       ///< Execution at the midpoint price.
    };

    /*!
     * @struct LevelData
     * @brief Contains information about order levels, including quantity and count.
     */
    struct LevelData {
        Quantity _quantity{}; ///< Total quantity at the level.
        Quantity _count{};    ///< Count of orders at the level.

        /*!
         * @enum Action
         * @brief Actions that can affect level metadata.
         */
        enum class Action {
            Add,    ///< Add a new order to the level.
            Remove, ///< Remove an order from the level.
            Match   ///< Match an order, partially or fully.
        };
    };

    std::map<Price, OrderPtrs, std::greater<Price>> _asks; ///< Ask orders sorted by price.
    std::map<Price, OrderPtrs, std::less<Price>> _bids; ///< Bid orders sorted by price.
    std::unordered_map<OrderId, OrderEntry> _orders; ///< All active orders by ID.
    std::unordered_map<Price, LevelData> _data; ///< Data about levels by price.

    mutable std::mutex _ordersMutex; ///< Mutex for synchronizing orderbook operations.
    std::thread _ordersPruneThread; ///< Thread for pruning Good For Day orders.
    std::condition_variable _shutdownConditionVariable; ///< Condition variable for shutdown signaling.
    std::atomic<bool> _shutdown{ false }; ///< Shutdown flag.

    ExecutionTypes _executionType = ExecutionTypes::BuyersPrice; ///< Current execution type.
    OrderDetailHistory _orderDetailHistory; ///< Tracks order and trade history.

    // Private member functions
    bool canMatch(Side side, Price price) const;
    Trades MatchOrders();
    void CancelOrders(OrderIds orderIds);
    void CancelOrderInternal(OrderId orderId);
    bool canFullyFill(Side side, Price price, Quantity quantity) const;

    void onOrderCanceleld(OrderPtr order);
    void onOrderAdded(OrderPtr order);
    void onOrderMatchedWithHistoryUpdate(Price price, Quantity quantity, bool isFullyFilled, Price purchasePrice);
    void onOrderMatched(Price price, Quantity quantity, bool isFullyFilled);
    void UpdateLevelData(Price price, Quantity quantity, LevelData::Action action);
    Trades MatchOrder(OrderModify order);

    // Private methods for managing Good For Day orders
    void PruneGoodForDayOrders();

public:
    /*!
     * @brief Default constructor, initializes the Good For Day order pruning thread.
     */
    Orderbook() : _ordersPruneThread{ [this] { PruneGoodForDayOrders(); } } {}

    // Deleted copy and move constructors and operators to prevent copying
    Orderbook(const Orderbook&) = delete;
    void operator=(const Orderbook&) = delete;
    Orderbook(Orderbook&&) = delete;
    void operator=(Orderbook&&) = delete;

    /*!
     * @brief Destructor, joins the pruning thread and signals shutdown.
     */
    ~Orderbook() {
        _shutdown.store(true, std::memory_order_release);
        _shutdownConditionVariable.notify_one();
        _ordersPruneThread.join();
    }

    /*!
     * @brief Changes the execution type for order matching.
     *
     * @param type The execution type (0: BuyersPrice, 1: SellersPrice, 2: MidPrice).
     */
    void changeExecutionType(int type);

    /*!
     * @brief Adds an order to the orderbook.
     *
     * @param order The order to add.
     * @return A vector of Trades resulting from the addition.
     */
    Trades addOrder(OrderPtr order);

    /*!
     * @brief Adds a new order to the orderbook with specified parameters.
     *
     * @param type The type of the order.
     * @param side The side of the order (Buy or Sell).
     * @param price The price of the order.
     * @param qty The quantity of the order.
     * @return A vector of Trades resulting from the addition.
     */
    Trades addOrder(OrderType type, Side side, Price price, Quantity qty);

    /*!
     * @brief Adds a new order with a specified ID.
     *
     * @param type The type of the order.
     * @param orderId The ID of the order.
     * @param side The side of the order (Buy or Sell).
     * @param price The price of the order.
     * @param qty The quantity of the order.
     * @return A vector of Trades resulting from the addition.
     */
    Trades addOrder(OrderType type, OrderId orderId, Side side, Price price, Quantity qty);

    /*!
     * @brief Adds an order to the orderbook via Python interface.
     *
     * @param type The type of the order (as integer).
     * @param id The ID of the order.
     * @param side The side of the order (1 for Buy, 2 for Sell).
     * @param price The price of the order.
     * @param qty The quantity of the order.
     */
    void addOrderViaPython(int type, int id, int side, int price, int qty);

    /*!
     * @brief Cancels an order by its ID.
     *
     * @param orderId The ID of the order to cancel.
     */
    void CancelOrder(OrderId orderId);

    /*!
     * @brief Modifies an existing order.
     *
     * @param order The OrderModify object containing new order details.
     * @return A vector of Trades resulting from the modification.
     */
    Trades ModifyOrder(OrderModify order);

    /*!
     * @brief Sets the price scale for the orderbook.
     *
     * @param _price The new price scale to set.
     */
    void setPriceScale(Price _price);

    /*!
     * @brief Retrieves the current price scale of the orderbook.
     *
     * @return The current price scale.
     */
    Price getPriceScale();

    /*!
     * @brief Prints all orders and orderbook details.
     */
    void printAllOrders();

    /*!
     * @brief Saves orderbook data to JSON files.
     *
     * Exports order histories and live orders to specified JSON files.
     *
     * @param buyfilename Filename for buy order history.
     * @param sellfilename Filename for sell order history.
     * @param purchasefilename Filename for purchase history.
     * @param buyLivefilename Filename for live buy orders.
     * @param sellLivefilename Filename for live sell orders.
     */
    void saveToJson(const std::string& buyfilename = "BuyHistory.json", const std::string& sellfilename = "SellHistory.json", const std::string& purchasefilename = "PurchaseHistory.json", const std::string& buyLivefilename = "BuyLiveHistory.json", const std::string& sellLivefilename = "SellLiveHistory.json");

    /*!
     * @brief Retrieves the number of orders in the orderbook.
     *
     * @return The size of the orderbook.
     */
    std::size_t Size() const;

    /*!
     * @brief Retrieves aggregated orderbook level information.
     *
     * Provides information about the bid and ask levels in the orderbook,
     * including prices and quantities.
     *
     * @return An OrderbookLevelInfos object containing level information.
     */
    OrderbookLevelInfos getOrderInfos() const;

    /*!
     * @brief Handles end-of-day processing for the orderbook.
     *
     * Cancels GoodForDay orders and cleans up orders and order entries.
     */
    void EndOfDay();
};
