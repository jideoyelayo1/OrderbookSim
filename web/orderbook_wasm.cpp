/*!
 * @file orderbook_wasm.cpp
 * @brief Embind binding: exposes Book to JavaScript with plain objects and arrays.
 *
 * Ids cross as doubles (exact below 2^53), not BigInt. Every returned object is a fresh JavaScript
 * value; only the Book itself is a C++ object and needs delete().
 */

#include "Book.hpp"

#include <emscripten/bind.h>

using emscripten::val;

namespace {

val levelsToJs(const LevelInfos& levels) {
    val out = val::array();
    for (const auto& level : levels) {
        val entry = val::object();
        entry.set("price", level._price);
        entry.set("quantity", static_cast<double>(level._quantity)); // a 64-bit sum; as a double, not a BigInt
        out.call<void>("push", entry);
    }
    return out;
}

val resultToJs(const BookResult& result) {
    val trades = val::array();
    for (const auto& trade : result.trades) {
        val entry = val::object();
        entry.set("bidId", static_cast<double>(trade.getBidTrade()._orderId));
        entry.set("bidPrice", trade.getBidTrade()._price);
        entry.set("askId", static_cast<double>(trade.getAskTrade()._orderId));
        entry.set("askPrice", trade.getAskTrade()._price);
        entry.set("quantity", trade.getBidTrade()._quantity);
        trades.call<void>("push", entry);
    }
    val out = val::object();
    out.set("id", static_cast<double>(result.id));
    out.set("status", result.status);
    out.set("restingQuantity", result.restingQuantity);
    out.set("trades", trades);
    return out;
}

} // namespace

EMSCRIPTEN_BINDINGS(orderbook) {
    using emscripten::optional_override;

    emscripten::class_<Book>("Book")
        .constructor<>()
        .function("add", optional_override([](Book& book, const std::string& type, const std::string& side, double price, double qty) {
            return resultToJs(book.add(type, side, price, qty));
        }))
        .function("cancel", &Book::cancel)
        .function("modify", optional_override([](Book& book, double id, const std::string& side, double price, double qty) {
            return resultToJs(book.modify(id, side, price, qty));
        }))
        .function("endOfDay", &Book::endOfDay)
        .function("levels", optional_override([](const Book& book) {
            const auto levels = book.levels();
            val out = val::object();
            out.set("bids", levelsToJs(levels.getBids()));
            out.set("asks", levelsToJs(levels.getAsks()));
            return out;
        }))
        .function("size", optional_override([](const Book& book) { return static_cast<double>(book.size()); }))
        .function("prediction", &Book::prediction)
        .function("recentError", &Book::recentError)
        .function("vwap", &Book::vwap)
        .function("reset", &Book::reset);
}
