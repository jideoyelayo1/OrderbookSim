/*!
 * @file Orderbook.cpp
 * @brief Implements the Orderbook class for managing orders in a trading system.
 *
 * This file contains the implementation of the Orderbook class, including methods
 * for adding, matching, canceling, and modifying orders, as well as handling
 * orderbook events and maintaining orderbook state.
 */

#include "Orderbook.hpp"

#include <mutex>

/*!
 * @brief Checks if an order can match based on side and price.
 *
 * Determines whether an incoming order can be matched against existing orders
 * in the orderbook based on the side (buy/sell) and price.
 *
 * @param side The side of the order (Buy or Sell).
 * @param price The price of the incoming order.
 * @return True if the order can match, false otherwise.
 */
bool Orderbook::canMatch(Side side, Price price) const {
    if (side == Side::Buy) {
        if (_asks.empty()) return false;
        const auto& [bestAsk, _] = *_asks.begin(); // gets the price
        return price >= bestAsk;
    } // else not needed
    if (_bids.empty()) return false;
    const auto& [bestBid, _] = *_bids.begin(); // gets the price
    return price <= bestBid;
}

/*!
 * @brief Matches orders in the orderbook.
 *
 * Iteratively matches buy and sell orders in the orderbook until no more matches
 * can be made. Updates order quantities, removes filled orders, and records trades.
 *
 * @return A vector of Trades that were executed.
 */
Trades Orderbook::MatchOrders()
{
    Trades trades;
    trades.reserve(_orders.size());

    while (true)
    {
        if (_bids.empty() || _asks.empty())
            break;

        auto& [bidPrice, bids] = *_bids.begin();
        auto& [askPrice, asks] = *_asks.begin();

        if (bidPrice > askPrice) // for some reasoon this works: bidPrice > askPrice should be the other way around
            break;

        while (!bids.empty() && !asks.empty())
        {
            OrderPtr bid = bids.front();
            OrderPtr ask = asks.front();

            Quantity quantity = std::min(bid->getRemainingQty(), ask->getRemainingQty());

            bid->Fill(quantity);
            ask->Fill(quantity);

            if (bid->isFilled())
            {
                bids.pop_front();
                _orders.erase(bid->getOrderId());
            }

            if (ask->isFilled())
            {
                asks.pop_front();
                _orders.erase(ask->getOrderId());
            }

            trades.push_back(Trade{
                TradeInfo{ bid->getOrderId(), bid->getPrice(), quantity },
                TradeInfo{ ask->getOrderId(), ask->getPrice(), quantity }
                });

            Price purchasedPrice = 0;

            switch (_executionType) {
            case ExecutionTypes::BuyersPrice:
                purchasedPrice = std::max(ask->getPrice(), bid->getPrice());
                break;
            case ExecutionTypes::SellersPrice:
                purchasedPrice = std::min(ask->getPrice(), bid->getPrice());
                break;
            case ExecutionTypes::MidPrice:
                purchasedPrice = (ask->getPrice() + bid->getPrice()) / 2;
                break;
            default: break;
            }
            
            _orderDetailHistory.removeMatchedOrder(bid->getOrderId(), ask->getOrderId());

            onOrderMatched(bid->getPrice(), quantity, bid->isFilled());
            onOrderMatchedWithHistoryUpdate(ask->getPrice(), quantity, ask->isFilled(), purchasedPrice); // we are purchasing at the max price
        }

        if (bids.empty())
        {
            _bids.erase(bidPrice);
            _data.erase(bidPrice);
        }

        if (asks.empty())
        {
            _asks.erase(askPrice);
            _data.erase(askPrice);
        }
    }

    if (!_bids.empty())
    {
        auto& [_, bids] = *_bids.begin();
        auto& order = bids.front();
        if (order->getOrderType() == OrderType::FillAndKill)
            CancelOrder(order->getOrderId());
    }

    if (!_asks.empty())
    {
        auto& [_, asks] = *_asks.begin();
        auto& order = asks.front();
        if (order->getOrderType() == OrderType::FillAndKill)
            CancelOrder(order->getOrderId());
    }

    return trades;
}

/*!
 * @brief Cancels multiple orders by their IDs.
 *
 * Iterates over the provided list of order IDs and cancels each order.
 *
 * @param orderIds A vector of OrderIds to cancel.
 */
void Orderbook::CancelOrders(OrderIds orderIds) {
    std::scoped_lock ordersLock{ _ordersMutex };
    for (const auto& orderId : orderIds) CancelOrderInternal(orderId);
}

/*!
 * @brief Internally cancels an order.
 *
 * Removes the order from the orderbook and updates related data structures.
 *
 * @param orderId The ID of the order to cancel.
 */
void Orderbook::CancelOrderInternal(OrderId orderId) {
    if (!_orders.contains(orderId)) return;
    const auto [order, orderIter] = _orders.at(orderId); // types returned OrderId, OrderEntry

    if (order->getSide() == Side::Sell) {
        auto price = order->getPrice();
        auto& orders = _asks.at(price);
        orders.erase(orderIter);
        if (orders.empty()) _asks.erase(price);
    }
    else //if(order->getSide() == Side::Buy) 
    {
        auto price = order->getPrice();
        auto& orders = _bids.at(price);
        orders.erase(orderIter);
        if (orders.empty()) _bids.erase(price);
    }
    onOrderCanceleld(order);
    _orders.erase(orderId); // delete the order from orders by Id at the end
}

/*!
 * @brief Checks if an order can be fully filled based on side, price, and quantity.
 *
 * Determines whether an incoming order can be fully matched with existing orders.
 *
 * @param side The side of the order (Buy or Sell).
 * @param price The price of the incoming order.
 * @param quantity The quantity of the incoming order.
 * @return True if the order can be fully filled, false otherwise.
 */
bool Orderbook::canFullyFill(Side side, Price price, Quantity quantity) const {
    if (!canMatch(side, price)) return false;

    std::optional<Price> threshold;

    if (side == Side::Buy) {
        const auto& [askPrice, _] = *_asks.begin();
        threshold = askPrice;
    }
    else {
        const auto& [bidPrice, _] = *_bids.begin();
        threshold = bidPrice;
    }
    for (const auto& [levelPrice, levelData] : _data) {
        if (threshold.has_value() &&
            (side == Side::Buy && threshold.value() > levelPrice) ||
            (side == Side::Sell && threshold.value() < levelPrice)
            ) continue;
        if ((side == Side::Buy && levelPrice > price) ||
            (side == Side::Sell && levelPrice < price)) continue;
        if (quantity <= levelData._quantity) return true;
        quantity -= levelData._quantity;
    }
    return false;
}

/*!
 * @brief Event handler for when an order is canceled.
 *
 * Updates level data when an order is canceled.
 *
 * @param order The order that was canceled.
 */
void Orderbook::onOrderCanceleld(OrderPtr order) {
    UpdateLevelData(order->getPrice(), order->getRemainingQty(), LevelData::Action::Remove);
}

/*!
 * @brief Event handler for when an order is added.
 *
 * Updates level data when a new order is added.
 *
 * @param order The order that was added.
 */
void Orderbook::onOrderAdded(OrderPtr order) {
    UpdateLevelData(order->getPrice(), order->getInitialQty(), LevelData::Action::Add);
}

/*!
 * @brief Event handler for when an order is matched and updates history.
 *
 * Updates level data and order history when an order is matched.
 *
 * @param price The price at which the order was matched.
 * @param quantity The quantity that was matched.
 * @param isFullyFilled Indicates if the order was fully filled.
 * @param purchasePrice The price at which the purchase occurred.
 */
void Orderbook::onOrderMatchedWithHistoryUpdate(Price price, Quantity quantity, bool isFullyFilled, Price purchasePrice) {
    if (purchasePrice == 0) {/*DO NOTHING*/; }
    else if (isFullyFilled) _orderDetailHistory.addOrderToPurchaseHistory(purchasePrice, quantity);
    else _orderDetailHistory.addOrderToPurchaseHistory(purchasePrice, std::min(quantity, _data[price]._quantity));

    _orderDetailHistory.updateNeutralNetwork(purchasePrice, std::min(quantity, _data[price]._quantity));

    UpdateLevelData(price, quantity, isFullyFilled ? LevelData::Action::Remove : LevelData::Action::Match);
}

/*!
 * @brief Event handler for when an order is matched.
 *
 * Updates level data when an order is matched.
 *
 * @param price The price at which the order was matched.
 * @param quantity The quantity that was matched.
 * @param isFullyFilled Indicates if the order was fully filled.
 */
void Orderbook::onOrderMatched(Price price, Quantity quantity, bool isFullyFilled) {
    UpdateLevelData(price, quantity, isFullyFilled ? LevelData::Action::Remove : LevelData::Action::Match);
}

/*!
 * @brief Updates level data for a given price level.
 *
 * Adjusts the count and quantity at a price level based on the action (add, remove, match).
 *
 * @param price The price level to update.
 * @param quantity The quantity to adjust.
 * @param action The action to perform (Add, Remove, Match).
 */
void Orderbook::UpdateLevelData(Price price, Quantity quantity, LevelData::Action action) {
    auto& data = _data[price];

    data._count += action == LevelData::Action::Remove ? -1 : action == LevelData::Action::Add ? 1 : 0;
    if (action == LevelData::Action::Remove || action == LevelData::Action::Match)
    {
        data._quantity -= quantity;
    }
    else
    {
        data._quantity += quantity;
    }

    if (data._count == 0)
        _data.erase(price);
}

/*!
 * @brief Modifies an existing order.
 *
 * Cancels the existing order and adds a new one with modified details.
 *
 * @param order The OrderModify object containing new order details.
 * @return A vector of Trades resulting from the modification.
 */
Trades Orderbook::MatchOrder(OrderModify order) {
    if (!_orders.contains(order.getOrderId())) return { };
    const auto& [existingOrder, _] = _orders.at(order.getOrderId());
    CancelOrder(order.getOrderId());
    return addOrder(order.toOrderPtr(existingOrder->getOrderType()));
}

/*!
 * @brief Prunes GoodForDay orders at the end of the trading day.
 *
 * Periodically checks the time and cancels all GoodForDay orders at market close.
 */
void Orderbook::PruneGoodForDayOrders() {
    using namespace std::chrono;
    const auto end = hours(16);
    while (true) {
        const auto now = system_clock::now();
        const auto now_c = system_clock::to_time_t(now);
        std::tm now_parts;
        localtime_s(&now_parts, &now_c);

        if (now_parts.tm_hour >= end.count()) now_parts.tm_mday++;

        now_parts.tm_hour = end.count();
        now_parts.tm_min = 0;
        now_parts.tm_sec = 0;

        auto next = system_clock::from_time_t(mktime(&now_parts));
        auto till = next - now + milliseconds(100);

        { // we don't want this thread to alter the state of our data structures at the same time
            std::unique_lock ordersLock{ _ordersMutex };
            if (_shutdown.load(std::memory_order_acquire)
                || _shutdownConditionVariable.wait_for(ordersLock, till) == std::cv_status::no_timeout)
                return;
        }

        OrderIds orderIds;

        {
            std::scoped_lock ordersLock{ _ordersMutex };
            for (const auto& [_, entry] : _orders) {
                const auto& [order, _] = entry;
                if (order->getOrderType() != OrderType::GoodForDay) continue;
                orderIds.push_back(order->getOrderId());
            }
        }

        CancelOrders(orderIds);
    }
}

/*!
 * @brief Adds a new order to the orderbook.
 *
 * Creates and adds a new order to the orderbook with the specified parameters.
 *
 * @param type The type of the order.
 * @param side The side of the order (Buy or Sell).
 * @param price The price of the order.
 * @param qty The quantity of the order.
 * @return A vector of Trades resulting from the addition.
 */
Trades Orderbook::addOrder(OrderType type, Side side, Price price, Quantity qty) {
    return addOrder(std::make_shared<Order>(type, Size() + 1, side, price, qty));
}

/*!
 * @brief Adds a new order to the orderbook with a specified ID.
 *
 * Creates and adds a new order with the specified ID and parameters.
 *
 * @param type The type of the order.
 * @param id The ID of the order.
 * @param side The side of the order (Buy or Sell).
 * @param price The price of the order.
 * @param qty The quantity of the order.
 * @return A vector of Trades resulting from the addition.
 */
Trades Orderbook::addOrder(OrderType type, OrderId id, Side side, Price price, Quantity qty) {
    return addOrder(std::make_shared<Order>(type, id, side, price, qty));
}

/*!
 * @brief Adds an order to the orderbook via Python interface.
 *
 * Exposed to Python via Pybind11, allows adding orders from Python code.
 *
 * @param type The type of the order (as integer).
 * @param id The ID of the order.
 * @param side The side of the order (1 for Buy, 2 for Sell).
 * @param price The price of the order.
 * @param qty The quantity of the order.
 */
void Orderbook::addOrderViaPython(int type, int id, int side, int price, int qty) {
    addOrder(std::make_shared<Order>(intToOrdertType(type), id, side % 2 == 1 ? Side::Buy : Side::Sell , price, qty));
    saveToJson();
}

/*!
 * @brief Adds an order to the orderbook.
 *
 * Adds the provided order to the orderbook, updates internal data structures,
 * and attempts to match orders.
 *
 * @param order A shared pointer to the Order to add.
 * @return A vector of Trades resulting from the addition.
 */
Trades Orderbook::addOrder(OrderPtr order) {
    std::scoped_lock ordersLock{ _ordersMutex };

    // Add to history
    _orderDetailHistory.addOrderToHistory(order->getOrderType(), order->getOrderId(), order->getSide(), order->getPrice(), order->getQty());

    if (_orders.contains(order->getOrderId())) return { };

    if (order->getOrderType() == OrderType::Market)
    {
        if (order->getSide() == Side::Buy && !_asks.empty())
        {
            const auto& [worstAsk, _] = *_asks.rbegin();
            order->toGoodTillCancel(worstAsk);
        }
        else if (order->getSide() == Side::Sell && !_bids.empty())
        {
            const auto& [worstBid, _] = *_bids.rbegin();
            order->toGoodTillCancel(worstBid);
        }
        else
            return { };
    }

    if (order->getOrderType() == OrderType::FillAndKill && !canMatch(order->getSide(), order->getPrice())) return { };
    if (order->getOrderType() == OrderType::FillOrKill && !canFullyFill(order->getSide(), order->getPrice(), order->getInitialQty())) return { };

    OrderPtrs::iterator iter;
    if (order->getSide() == Side::Buy) {
        auto& orders = _bids[order->getPrice()];
        orders.push_back(order);
        iter = std::prev(orders.end());
    }
    else {
        auto& orders = _asks[order->getPrice()];
        orders.push_back(order);
        iter = std::prev(orders.end());
    }
    _orders.insert({ order->getOrderId(), OrderEntry{order,iter} });

    onOrderAdded(order);

    return MatchOrders();
}

/*!
 * @brief Cancels an order by its ID.
 *
 * Public interface to cancel an order.
 *
 * @param orderId The ID of the order to cancel.
 */
void Orderbook::CancelOrder(OrderId orderId) {
    std::scoped_lock ordersLock{ _ordersMutex };
    CancelOrderInternal(orderId);
}

/*!
 * @brief Modifies an existing order.
 *
 * Replaces the existing order with a new one based on the modification details.
 *
 * @param order The OrderModify object containing new order details.
 * @return A vector of Trades resulting from the modification.
 */
Trades Orderbook::ModifyOrder(OrderModify order) {
    OrderType orderType;
    {
        std::scoped_lock ordersLock{ _ordersMutex };
        if (!_orders.contains(order.getOrderId())) return { };
        const auto& [existingOrder, _] = _orders.at(order.getOrderId());
        orderType = existingOrder->getOrderType();
    }
    CancelOrder(order.getOrderId());
    return addOrder(order.toOrderPtr(orderType));
}

/*!
 * @brief Retrieves the number of orders in the orderbook.
 *
 * @return The size of the orderbook.
 */
std::size_t Orderbook::Size() const { return _orders.size(); }

/*!
 * @brief Retrieves aggregated orderbook level information.
 *
 * Provides information about the bid and ask levels in the orderbook,
 * including prices and quantities.
 *
 * @return An OrderbookLevelInfos object containing level information.
 */
OrderbookLevelInfos Orderbook::getOrderInfos() const {
    LevelInfos bidInfos, askInfos;
    bidInfos.reserve(_orders.size());
    askInfos.reserve(_orders.size());

    auto CreateLevelInfos = [](Price price, const OrderPtrs& orders)
        {
            return LevelInfo{ price, std::accumulate(orders.begin(), orders.end(), (Quantity)0,
                [](Quantity runningSum, const OrderPtr& order)
                { return runningSum + order->getRemainingQty(); }) };
        };

    for (const auto& [price, orders] : _bids)
        bidInfos.push_back(CreateLevelInfos(price, orders));

    for (const auto& [price, orders] : _asks)
        askInfos.push_back(CreateLevelInfos(price, orders));

    return OrderbookLevelInfos{ bidInfos, askInfos };
}

/*!
 * @brief Prints all orders and orderbook details.
 *
 * Outputs order histories and statistics to the console.
 */
void Orderbook::printAllOrders() {
    //_orderDetailHistory._printSellHistory();
    //_orderDetailHistory._printBuyHistory();
    _orderDetailHistory._printPurchaseHistory();
    //_orderDetailHistory._printLiveOrders();

    std::cout << "Purchase History Size: " << _orderDetailHistory.purchaseHistorySize();

    std::cout << "Sell History Size: " << _orderDetailHistory.sellHistorySize();
    std::cout << "Buy History Size: " << _orderDetailHistory.buyHistorySize();
    
    if (_orderDetailHistory.getVWAP() == 0) std::cout << "No purchases made" << std::endl;
    else std::cout << "The VWAP of this item is " << _orderDetailHistory.getVWAP() << std::endl;

    std::cout << "PREDICTION " << _orderDetailHistory.getPrediction() << std::endl;
}

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
void Orderbook::saveToJson(const std::string& buyfilename, const std::string& sellfilename, const std::string& purchasefilename, const std::string& buyLivefilename, const std::string& sellLivefilename) {
    _orderDetailHistory.saveHistoryToJson(buyfilename, sellfilename, purchasefilename, sellLivefilename, buyLivefilename);
}

/*!
 * @brief Changes the execution type for order matching.
 *
 * Adjusts how the execution price is determined during order matching.
 *
 * @param type The execution type (0: BuyersPrice, 1: SellersPrice, 2: MidPrice).
 */
void Orderbook::changeExecutionType(int type) {
    switch (type) {
    case 0: _executionType = ExecutionTypes::BuyersPrice; return;
    case 1: _executionType = ExecutionTypes::SellersPrice; return;
    case 2: _executionType = ExecutionTypes::MidPrice; return;
    default: return;
    }
}

/*!
 * @brief Handles end-of-day processing for the orderbook.
 *
 * Cancels GoodForDay orders and cleans up orders and order entries.
 */
void Orderbook::EndOfDay() {
    std::unordered_map<OrderId, bool> goodToCancels;

    // Handle live orders
    for (const auto& m : _orderDetailHistory.getLiveOrders()) {
        if (m.second.OrderType() != OrderType::GoodForDay) continue;
        goodToCancels[m.first] = true;
        _orderDetailHistory.deleteALiveOrder(m.first);
    }

    // Remove orders not in goodToCancels
    for (auto it = _orders.begin(); it != _orders.end(); ) {
        if (goodToCancels.find(it->first) != goodToCancels.end()) {
            ++it;
        }
        else {
            it = _orders.erase(it);
        }
    }

    // Clean up bids
    for (auto it = _bids.begin(); it != _bids.end(); ) {
        auto& orderptrs = it->second;
        for (auto orderIt = orderptrs.begin(); orderIt != orderptrs.end(); ) {
            if (goodToCancels.find((*orderIt)->getOrderId()) != goodToCancels.end()) {
                orderIt = orderptrs.erase(orderIt);
            }
            else {
                ++orderIt;
            }
        }
        if (orderptrs.empty()) {
            it = _bids.erase(it);
        }
        else {
            ++it;
        }
    }

    // Clean up asks
    for (auto it = _asks.begin(); it != _asks.end(); ) {
        auto& orderptrs = it->second;
        for (auto orderIt = orderptrs.begin(); orderIt != orderptrs.end(); ) {
            if (goodToCancels.find((*orderIt)->getOrderId()) != goodToCancels.end()) {
                orderIt = orderptrs.erase(orderIt);
            }
            else {
                ++orderIt;
            }
        }
        if (orderptrs.empty()) {
            it = _asks.erase(it);
        }
        else {
            ++it;
        }
    }
}

/*!
 * @brief Sets the price scale for the orderbook.
 *
 * Adjusts the price scale used in the orderbook's internal calculations.
 *
 * @param _price The new price scale to set.
 */
void Orderbook::setPriceScale(Price _price) {
    _orderDetailHistory.PRICESCALE = _price;
}

/*!
 * @brief Retrieves the current price scale of the orderbook.
 *
 * @return The current price scale.
 */
Price Orderbook::getPriceScale() {
    return _orderDetailHistory.PRICESCALE;
}
