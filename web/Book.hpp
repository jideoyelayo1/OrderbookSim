/*!
 * @file Book.hpp
 * @brief Defines Book, the Orderbook wrapper behind the JavaScript binding.
 *
 * Book assigns its own order ids, validates arguments arriving from JavaScript and reports
 * what happened to each order. It has no Emscripten dependency, so the native tests and the
 * parity driver use the same class as orderbook_wasm.cpp.
 *
 * Prices and quantities are whole numbers from 1 to 1,000,000 (Book::maxPrice, Book::maxQuantity).
 * The cap keeps one trade's price * quantity at or below 10^12, so the 64-bit VWAP sums stay exact
 * for 9,223,372 trades even at the maximum price and quantity, and it keeps the quantity the
 * network receives as an int far from overflow. A level's total is a 64-bit sum, so no cap on
 * the number of resting orders is needed.
 */

#pragma once

#include "Orderbook.hpp"

#include <cmath>
#include <memory>
#include <optional>
#include <string>
#include <utility>

/*!
 * @struct BookResult
 * @brief The outcome of Book::add or Book::modify.
 *
 * status is "filled" (all of the quantity traded), "partial" (some traded), "resting" (none
 * traded, the order rests), "rejected" (none traded, nothing rests) or "invalid" (bad
 * arguments; the book is untouched and id is 0). restingQuantity is how much of the order
 * is left in the book: 0 after a Fill-and-Kill partial fill, whose remainder is cancelled.
 */
struct BookResult {
    OrderId id = 0;               ///< The order's id, 0 when invalid.
    std::string status;           ///< filled, partial, resting, rejected or invalid.
    Quantity restingQuantity = 0; ///< Quantity of this order left in the book.
    Trades trades;                ///< Trades this call made, in execution order.
};

/*!
 * @class Book
 * @brief An Orderbook that numbers its own orders and reports each order's outcome.
 *
 * Not thread-safe: the id counter and the check of whether an order still rests after a call
 * are not locked, so use each Book from one thread at a time (the WebAssembly module has one).
 */
class Book {
public:
    static constexpr double maxPrice = 1'000'000;    ///< Largest price add and modify accept.
    static constexpr double maxQuantity = 1'000'000; ///< Largest quantity add and modify accept.

private:
    std::unique_ptr<Orderbook> _book = std::make_unique<Orderbook>(); ///< The order book.
    OrderId _nextId = 1; ///< Id of the next valid add; never reused, unlike Size() + 1.

    /*!
     * @brief Checks that a value from JavaScript is a whole number in [low, high]; NaN fails.
     */
    static bool isWhole(double value, double low, double high) {
        return value >= low && value <= high && std::floor(value) == value;
    }

    static std::optional<OrderType> parseType(const std::string& type) {
        for (const auto candidate : { OrderType::GoodTillCancel, OrderType::FillAndKill, OrderType::Market, OrderType::GoodForDay, OrderType::FillOrKill })
            if (type == orderTypeToString(candidate)) return candidate;
        return std::nullopt;
    }

    static std::optional<Side> parseSide(const std::string& side) {
        if (side == "Buy") return Side::Buy;
        if (side == "Sell") return Side::Sell;
        return std::nullopt;
    }

    static bool isPrice(double price) { return isWhole(price, 1, maxPrice); }
    static bool isQuantity(double qty) { return isWhole(qty, 1, maxQuantity); }
    static bool isId(double id) { return isWhole(id, 1, 9007199254740992.0); } // ids cross to JavaScript as doubles: exact up to 2^53

    /*!
     * @brief Derives the outcome from the trades and whether the order still rests.
     */
    BookResult outcome(OrderId id, Quantity qty, Trades trades) const {
        Quantity traded = 0;
        for (const auto& trade : trades)
            if (trade.getBidTrade()._orderId == id || trade.getAskTrade()._orderId == id) traded += trade.getBidTrade()._quantity;
        const bool rests = _book->hasOrder(id);
        const char* status = traded == qty ? "filled" : traded > 0 ? "partial" : rests ? "resting" : "rejected";
        return BookResult{ id, status, rests ? qty - traded : 0, std::move(trades) };
    }

public:
    /*!
     * @brief Adds an order.
     *
     * @param type GoodTillCancel, FillAndKill, Market, GoodForDay or FillOrKill.
     * @param side Buy or Sell.
     * @param price Limit price, a whole number from 1 to maxPrice; ignored for Market orders.
     * @param qty Quantity, a whole number from 1 to maxQuantity.
     * @return The order's id and outcome; invalid arguments do not use up an id.
     */
    BookResult add(const std::string& type, const std::string& side, double price, double qty) {
        const auto orderType = parseType(type);
        const auto orderSide = parseSide(side);
        if (!orderType || !orderSide || !isQuantity(qty) || (*orderType != OrderType::Market && !isPrice(price)))
            return BookResult{ 0, "invalid", 0, {} };

        const OrderId id = _nextId++;
        const auto quantity = static_cast<Quantity>(qty);
        auto order = *orderType == OrderType::Market
            ? std::make_shared<Order>(id, *orderSide, quantity)
            : std::make_shared<Order>(*orderType, id, *orderSide, static_cast<Price>(price), quantity);
        return outcome(id, quantity, _book->addOrder(order));
    }

    /*!
     * @brief Cancels a resting order.
     *
     * @return True if the order was resting and has been removed.
     */
    bool cancel(double id) {
        if (!isId(id) || !_book->hasOrder(static_cast<OrderId>(id))) return false;
        _book->CancelOrder(static_cast<OrderId>(id));
        return true;
    }

    /*!
     * @brief Replaces a resting order, keeping its id and type; it loses time priority.
     *
     * @return The outcome as for add; "rejected" if the id is not resting.
     */
    BookResult modify(double id, const std::string& side, double price, double qty) {
        const auto orderSide = parseSide(side);
        if (!isId(id) || !orderSide || !isPrice(price) || !isQuantity(qty)) return BookResult{ 0, "invalid", 0, {} };

        const auto orderId = static_cast<OrderId>(id);
        if (!_book->hasOrder(orderId)) return BookResult{ orderId, "rejected", 0, {} };
        const auto quantity = static_cast<Quantity>(qty);
        return outcome(orderId, quantity, _book->ModifyOrder(OrderModify{ orderId, *orderSide, static_cast<Price>(price), quantity }));
    }

    /*!
     * @brief Cancels every Good-For-Day order.
     */
    void endOfDay() { _book->EndOfDay(); }

    /*!
     * @brief Aggregated levels, best first on each side.
     */
    OrderbookLevelInfos levels() const { return _book->getOrderInfos(); }

    /*!
     * @brief Number of resting orders.
     */
    std::size_t size() const { return _book->Size(); }

    Price prediction() const { return _book->getPrediction(); }
    double recentError() const { return _book->getRecentAverageError(); }
    double vwap() const { return _book->getVWAP(); }

    /*!
     * @brief Replaces the book with an empty one, with a new network, and restarts ids at 1.
     */
    void reset() {
        _book = std::make_unique<Orderbook>();
        _nextId = 1;
    }
};
