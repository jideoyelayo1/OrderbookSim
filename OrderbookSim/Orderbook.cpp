#include "Orderbook.h"

#include <mutex>

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
            auto bid = bids.front();
            auto ask = asks.front();

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

            onOrderMatched(bid->getPrice(), quantity, bid->isFilled());
            onOrderMatchedWithHistoryUpdate(ask->getPrice(), quantity, ask->isFilled(),std::max(ask->getPrice(),bid->getPrice())); // we are purchasing at the max price
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
void Orderbook::CancelOrders(OrderIds orderIds) {
    std::scoped_lock ordersLock{ _ordersMutex };
    for (const auto& orderId : orderIds) CancelOrderInternal(orderId);
}
void Orderbook::CancelOrderInternal(OrderId orderId) { // You would have to take the mutex a lot of times wasting time (only taking the mutex one)
    if (!_orders.contains(orderId)) return;
    const auto [order, orderIter] = _orders.at(orderId);

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
bool Orderbook::canFullyFill(Side side, Price price, Quantity quantity) const { // if the order is fully filled we need to remove the order count from our bookkeeping DS, if not fully filled is does not need to be touched
    if (!canMatch(side, price)) return false; // does this order cause a cross

    std::optional<Price>threshold;

    if (side == Side::Buy) { // if it can match and its a buy we know theres at least one sell
        const auto& [askPrice, _] = *_asks.begin();
        threshold = askPrice;

    }
    else {// if it can match and its a sell we know that bids exists
        const auto& [bidPrice, _] = *_bids.begin();
        threshold = bidPrice;
    }
    for (const auto& [levelPrice, levelData] : _data) { // we care about the quantity in the range of the best ask and the ask the person wants: filter for at or below that levelPrice
        if (threshold.has_value() &&
            (side == Side::Buy && threshold.value() > levelPrice) ||
            (side == Side::Sell && threshold.value() < levelPrice)
            ) continue;
        if ((side == Side::Buy && levelPrice > price) ||
            (side == Side::Sell && levelPrice < price)) continue;
        //^ filtering in out everything we dont need first
        if (quantity <= levelData._quantity) return true; // we're finding the acculate and seeing if we can find enough quantites 
        quantity -= levelData._quantity;
    }
    return false;

}

// Private Event handlers (event APIs): revelent for fill or kill
void Orderbook::onOrderCanceleld(OrderPtr order) {
    UpdateLevelData(order->getPrice(), order->getRemainingQty(), LevelData::Action::Remove);
}
void Orderbook::onOrderAdded(OrderPtr order) {
    UpdateLevelData(order->getPrice(), order->getInitialQty(), LevelData::Action::Add);
}

void Orderbook::onOrderMatchedWithHistoryUpdate(Price price, Quantity quantity, bool isFullyFilled, Price purchasePrice) {

    if (isFullyFilled) _orderDetailHistory.addOrderToPurchaseHistory(purchasePrice, quantity);
    else _orderDetailHistory.addOrderToPurchaseHistory(purchasePrice, std::min(quantity, _data[price]._quantity));

    UpdateLevelData(price, quantity, isFullyFilled ? LevelData::Action::Remove : LevelData::Action::Match);

}
void Orderbook::onOrderMatched(Price price, Quantity quantity, bool isFullyFilled) {

    UpdateLevelData(price, quantity, isFullyFilled ? LevelData::Action::Remove : LevelData::Action::Match);

}

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

Trades Orderbook::MatchOrder(OrderModify order) {
    if (!_orders.contains(order.getOrderId())) return { };
    const auto& [existingOrder, _] = _orders.at(order.getOrderId());
    CancelOrder(order.getOrderId());
    return addOrder(order.toOrderPtr(existingOrder->getOrderType()));
}


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

        { // we dont want this thread to alter the state of out DS at the same time: Undefined behaviour
            std::unique_lock ordersLock{ _ordersMutex };
            if (_shutdown.load(std::memory_order_acquire) // if orderbook is no longer in use run
                || _shutdownConditionVariable.wait_for(ordersLock, till) == std::cv_status::no_timeout) // if signed a shutdown return
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

Trades Orderbook::addOrder(OrderType type, Side side, Price price, Quantity qty) {
    return addOrder(std::make_shared<Order>(type, Size() + 1, side, price, qty));

}

Trades Orderbook::addOrder(OrderType type, OrderId id, Side side, Price price, Quantity qty) {
    return addOrder(std::make_shared<Order>(type, id, side, price, qty));

}

Trades Orderbook::addOrder(OrderPtr order) {
    std::scoped_lock ordersLock{ _ordersMutex };

    // add to history
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
    if (order->getOrderType() == OrderType::FillOrKill && !canFullyFill(order->getSide(), order->getPrice(), order->getInitialQty())) return { }; // if it can not fill all then ignore order

    OrderPtrs::iterator iter;
    if (order->getSide() == Side::Buy) {
        auto& orders = _bids[order->getPrice()];
        orders.push_back(order);
        iter = std::next(orders.begin(), orders.size() - 1);
    }
    else {
        auto& orders = _asks[order->getPrice()];
        orders.push_back(order);
        iter = std::next(orders.begin(), orders.size() - 1);
    }
    _orders.insert({ order->getOrderId(),OrderEntry{order,iter} });
    //TODO:Check to see where it goes to size > 9 in both
    //if (_orders.size() > 9) throw std::logic_error("here");
    onOrderAdded(order);

    return MatchOrders();
}


void Orderbook::CancelOrder(OrderId orderId) {
    std::scoped_lock ordersLock{ _ordersMutex };
    CancelOrderInternal(orderId);

}

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


std::size_t Orderbook::Size() const { return _orders.size(); }

// Orderbook levels info object to represnet the aggreatate internal state of order book: useful for testing or market data decimination
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


void Orderbook::printAllOrders() {
    _orderDetailHistory._printSellHistory();
    _orderDetailHistory._printBuyHistory();
    _orderDetailHistory._printPurchaseHistory();
    if (_orderDetailHistory.getVWAP() == 0) std::cout << "No purchases made" << std::endl;
    else std::cout << "The VWAP of this item is " << _orderDetailHistory.getVWAP() << std::endl;
}

void Orderbook::saveToJson(const std::string& buyfilename, const std::string& sellfilename, const std::string& purchasefilename) {
    _orderDetailHistory.saveHistoryToJson(buyfilename, sellfilename, purchasefilename);
}