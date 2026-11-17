/*!
 * @file orderbook_wasm.cpp
 * @brief Embind binding: exposes Book to JavaScript with plain objects and arrays.
 *
 * Ids cross as doubles (exact below 2^53), not BigInt. Every returned object is a fresh JavaScript
 * value; only the Book itself is a C++ object and needs delete().
 *
 * Arguments arrive as val and are checked for their JavaScript type: a number argument must be a
 * JavaScript number and a string argument a JavaScript string. Anything else (a numeric string,
 * a boolean, null, undefined, a BigInt) reaches Book as NaN or "" and so gives "invalid" or false,
 * rather than being coerced (embind's double would turn '5' into 5 and true into 1) or throwing.
 */

#include "Book.hpp"

#include <emscripten/bind.h>

#include <cmath>
#include <string>

using emscripten::val;

namespace {

double number(const val& value) { return value.isNumber() ? value.as<double>() : std::nan(""); }
std::string text(const val& value) { return value.isString() ? value.as<std::string>() : std::string{}; }

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
        .function("add", optional_override([](Book& book, val type, val side, val price, val qty) {
            return resultToJs(book.add(text(type), text(side), number(price), number(qty)));
        }))
        .function("cancel", optional_override([](Book& book, val id) { return book.cancel(number(id)); }))
        .function("modify", optional_override([](Book& book, val id, val side, val price, val qty) {
            return resultToJs(book.modify(number(id), text(side), number(price), number(qty)));
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
