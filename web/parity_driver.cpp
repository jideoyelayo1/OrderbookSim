/*!
 * @file parity_driver.cpp
 * @brief Native side of the parity test (web/test.mjs).
 *
 * Reads one call per line from stdin, runs it through Book and prints one line per call, in the
 * format test.mjs produces from the WebAssembly module, so the two outputs can be compared as text.
 */

#include "Book.hpp"

#include <format>
#include <iostream>
#include <sstream>
#include <string>

namespace {

std::string tradesText(const Trades& trades) {
    if (trades.empty()) return "-";
    std::string out;
    for (const auto& trade : trades) {
        if (!out.empty()) out += ',';
        out += std::format("{}:{}/{}:{}/{}", trade.getBidTrade()._orderId, trade.getBidTrade()._price,
            trade.getAskTrade()._orderId, trade.getAskTrade()._price, trade.getBidTrade()._quantity);
    }
    return out;
}

std::string levelsText(const LevelInfos& levels) {
    if (levels.empty()) return "-";
    std::string out;
    for (const auto& level : levels) {
        if (!out.empty()) out += ',';
        out += std::format("{}:{}", level._price, level._quantity);
    }
    return out;
}

std::string resultText(const std::string& call, const BookResult& result) {
    return std::format("{} {} {} {} {}", call, result.id, result.status, result.restingQuantity, tradesText(result.trades));
}

} // namespace

int main() {
    Book book;
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream in(line);
        std::string call, type, side;
        double id = 0, price = 0, qty = 0;
        in >> call;
        if (call == "add" && in >> type >> side >> price >> qty) std::cout << resultText(call, book.add(type, side, price, qty)) << '\n';
        else if (call == "modify" && in >> id >> side >> price >> qty) std::cout << resultText(call, book.modify(id, side, price, qty)) << '\n';
        else if (call == "cancel" && in >> id) std::cout << "cancel " << (book.cancel(id) ? "true" : "false") << '\n';
        else if (call == "eod") { book.endOfDay(); std::cout << "eod " << book.size() << '\n'; }
        else if (call == "reset") { book.reset(); std::cout << "reset\n"; }
        else if (call == "levels") {
            const auto levels = book.levels();
            std::cout << "levels " << levelsText(levels.getBids()) << " | " << levelsText(levels.getAsks()) << '\n';
        }
        else if (call == "readouts") std::cout << std::format("readouts {} {} network {} {}\n", book.size(), book.vwap(), book.prediction(), book.recentError());
        else {
            std::cerr << "cannot parse call: " << line << '\n';
            return 1;
        }
    }
    return 0;
}
