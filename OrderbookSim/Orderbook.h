#pragma once

#include "Order.h"
#include "Trade.h"
#include "Usings.h"
#include "Order.h"
#include "OrderModify.h"
#include "OrderbookLevelInfos.h"

#include <map>
#include <unordered_map>
#include <optional>
#include <mutex>
#include <condition_variable>



class Orderbook {
private:
    // using a map to represent the bids and asks
    struct OrderEntry {
        OrderPtr _order{ nullptr };
        OrderPtrs::iterator _location;
    };
    struct LevelData {
        Quantity _quantity{};
        Quantity _count{};
        enum class Action { // how can metadara be impacted
            Add, // add new order to level
            Remove, // remove a order from level: remove quantity remove count
            Match, // A fully match: affects count and quantity, Partial match: order remains but we only impact quantity
        };
    };
    std::map<Price, OrderPtrs, std::greater<Price>> _asks;
    std::map<Price, OrderPtrs, std::less<Price>> _bids;
    std::unordered_map<OrderId, OrderEntry> _orders;
    std::unordered_map<Price, LevelData> _data;
    mutable std::mutex _ordersMutex;
    std::thread _ordersPruneThread;
    std::condition_variable _shutdownConditionVariable;
    std::atomic<bool> _shutdown{ false };

    bool canMatch(Side side, Price price) const;
    Trades MatchOrders();
    void CancelOrders(OrderIds orderIds);
    void CancelOrderInternal(OrderId orderId);
    bool canFullyFill(Side side, Price price, Quantity quantity) const;

    // Private Event handlers (event APIs): revelent for fill or kill
    void onOrderCanceleld(OrderPtr order);
    void onOrderAdded(OrderPtr order);
    void onOrderMatched(Price price, Quantity quantity, bool isFullyFilled);
    void UpdateLevelData(Price price, Quantity quantity, LevelData::Action action);

    Trades MatchOrder(OrderModify order);


public:
    Orderbook() :_ordersPruneThread{ [this] {PruneGoodForDayOrders(); } } {}
    Orderbook(const Orderbook&) = delete;
    void operator=(const Orderbook&) = delete;
    Orderbook(Orderbook&&) = delete;
    void operator=(Orderbook&&) = delete;
    ~Orderbook() {
        _shutdown.store(true, std::memory_order_release);
        _shutdownConditionVariable.notify_one();
        _ordersPruneThread.join();
    }

    void PruneGoodForDayOrders();

    Trades addOrder(OrderPtr order);
    void CancelOrder(OrderId orderId);
    Trades ModifyOrder(OrderModify order);




    std::size_t Size() const;

    // Orderbook levels info object to represnet the aggreatate internal state of order book: useful for testing or market data decimination
    OrderbookLevelInfos getOrderInfos() const;

};

